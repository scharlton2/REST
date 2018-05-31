#include "restdialog.h"
#include "ui_restdialog.h"

#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>


RESTDialog::RESTDialog(QWidget *parent) :
    QWidget(parent),
    m_manager(nullptr),
    ui(new Ui::RESTDialog)
{
    ui->setupUi(this);
    ui->siteLineEdit->setText("394329104490101");  // TOLL GATE CREEK ABOVE 6TH AVE AT AURORA, CO
    ui->startDateTimeEdit->setDateTime(QDateTime::fromString("2017-06-01T06:00", "yyyy-MM-ddThh:mm"));
    ui->endDateTimeEdit->setDateTime(QDateTime::fromString("2017-06-01T10:00", "yyyy-MM-ddThh:mm"));
    m_manager = new QNetworkAccessManager(this);
}

RESTDialog::~RESTDialog()
{
    delete ui;
}

void RESTDialog::on_genURLButton_clicked()
{
    static const char *formats[] = {"waterml,2.0", "waterml,2.0", "rdb", "json"};
    char buffer[32767];

    QApplication::setOverrideCursor(Qt::WaitCursor);

    /*
    68478       00060        00003      69928          Discharge, cubic feet per second (Mean)
    68511       00400        00008      69941          pH, water, unfiltered, field, standard units (Median), From multiparameter sonde

    00060  Discharge, cubic feet per second
    00065  Gage height, feet
    30207  Gage height, above datum, meters
    30208  Discharge, cubic meters per second
    */

    qsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]),
            "https://waterservices.usgs.gov/nwis/iv/?format=%s&sites=%s&startDT=%s&endDT=%s&parameterCd=00060,00065,30207,30208&siteStatus=all",
            formats[ui->formatComboBox->currentIndex()],
            ui->siteLineEdit->text().toStdString().c_str(),
            ui->startDateTimeEdit->dateTime().toString(Qt::ISODate).toStdString().c_str(),
            ui->endDateTimeEdit->dateTime().toString(Qt::ISODate).toStdString().c_str()
            );

    ui->urlEdit->setText(buffer);
    ui->urlEdit->selectAll();

    QUrl url(buffer);
    QNetworkRequest request;
    QNetworkReply *reply;

    while (url.toString().size() != 0) {
        // make request
        request.setUrl(url);
        reply = m_manager->get(request);

        // wait until finished
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        // HTTP status code
        // see https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
        // see https://waterservices.usgs.gov/rest/IV-Service.html#Error
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << status << reason.toStdString().c_str();

        // check for errors
        if (reply->error() != 0) break;

        // get redirect url (if any)
        url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    }

    if (reply->error() == 0) {
		if (ui->formatComboBox->currentIndex() != 0) {
			ui->outputPlainTextEdit->document()->setPlainText(reply->readAll());
		}
		else {
			QDomDocument doc;
			doc.setContent(reply->readAll());
			Q_ASSERT(!doc.isNull());

			QString lines;
			lines += "*******************\n";
			lines += "Parsed WaterML, 2.0\n";
			lines += "*******************\n";
			QDomNodeList list = doc.elementsByTagName("wml2:observationMember");
			for (int i = 0; i < list.count(); ++i) {
				QDomElement oMemb = list.at(i).toElement();
				QDomNodeList oProp = oMemb.elementsByTagName("om:observedProperty");
				Q_ASSERT(oProp.size() == 1);

				// property
				QString property;
				if (oProp.at(0).attributes().contains("xlink:title")) {
					property = oProp.at(0).attributes().namedItem("xlink:title").nodeValue();
					Q_ASSERT(property == "Discharge");
				}

				// property units
				QString units;
				QDomNodeList uom = oMemb.elementsByTagName("wml2:uom");
				if (uom.at(0).attributes().contains("xlink:title")) {
					units = uom.at(0).attributes().namedItem("xlink:title").nodeValue();
					Q_ASSERT(units == "ft3/s");
				}

				// headings
				lines += "Date/Time\t";
				lines += "Time Diff(s)\t";
				lines += property + "(" + units + ")\t";
				lines += property + "(m3/s)\n";

				// result
				QDomElement result = oMemb.elementsByTagName("om:result").at(0).toElement();

				// points
				QDomNodeList pts = result.elementsByTagName("wml2:point");
				QDateTime first, current;
				for (int p = 0; p < pts.count(); ++p) {
					// time
					QString time = pts.at(p).toElement().elementsByTagName("wml2:time").at(0).childNodes().at(0).toText().data();
					current = QDateTime::fromString(time, Qt::ISODate);
					if (p == 0) first = current;
					qint64 seconds = first.msecsTo(current) / 1000;

					// value
					QString value = pts.at(p).toElement().elementsByTagName("wml2:value").at(0).childNodes().at(0).toText().data();
					double ft3_per_sec = value.toDouble();
					double m3_per_sec = ft3_per_sec * 0.0283168;

					lines += time + "\t";
					lines += QString::number(seconds) + "\t";
					lines += value + "\t";
					lines += QString::number(m3_per_sec, 'g', 3) + "\n";
				}
			}
			ui->outputPlainTextEdit->document()->setPlainText(lines);
		}
    }
    else {
        ui->outputPlainTextEdit->document()->setPlainText(reply->errorString());
    }

    QApplication::restoreOverrideCursor();
}
