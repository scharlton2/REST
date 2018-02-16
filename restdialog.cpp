#include "restdialog.h"
#include "ui_restdialog.h"

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
    ui->endDateTimeEdit->setDateTime(QDateTime::fromString("2017-06-01T18:00", "yyyy-MM-ddThh:mm"));
    m_manager = new QNetworkAccessManager(this);
}

RESTDialog::~RESTDialog()
{
    delete ui;
}

void RESTDialog::on_genURLButton_clicked()
{
    static char *formats[] = {"waterml,2.0", "rdb", "json"};
    char buffer[32767];

    QApplication::setOverrideCursor(Qt::WaitCursor);

    qsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]),
            "https://waterservices.usgs.gov/nwis/iv/?format=%s&sites=%s&startDT=%s&endDT=%s&parameterCd=00060,00065&siteStatus=all",
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
        switch (status) {
        case 200:
            // ok
            qDebug() << "200 OK";
            break;
        case 301:
            // redirect (not considered an error)
            qDebug() << "301 Moved Permanently";
            break;
        case 400:
            // bad data
            qDebug() << "400 Bad Request";
            break;
        case 404:
            // invalid data
            qDebug() << "404 Not Found";
            break;
        default:
            qDebug() << status << " Unknown";
            break;
        }

        // check for errors
        if (reply->error() != 0) break;

        // get redirect url (if any)
        url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    }

    if (reply->error() == 0) {
        ui->outputPlainTextEdit->document()->setPlainText(reply->readAll());
    }
    else {
        ui->outputPlainTextEdit->document()->setPlainText(reply->errorString());
    }

    QApplication::restoreOverrideCursor();
}
