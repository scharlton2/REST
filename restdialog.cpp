#include "restdialog.h"
#include "ui_restdialog.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QThread>

#include <stdio.h>

RESTDialog::RESTDialog(QWidget *parent) :
    QWidget(parent),
    m_manager(nullptr),
    ui(new Ui::RESTDialog)
{
    ui->setupUi(this);
    ui->siteLineEdit->setText("394329104490101");  // TOLL GATE CREEK ABOVE 6TH AVE AT AURORA, CO
    //ui->siteLineEdit->setText("01646500");         // POTOMAC RIVER NEAR WASH, DC LITTLE FALLS PUMP STA
    QDateTime::fromString("2017-06-01", "yyyy-MM-dd");
    ui->startDateTimeEdit->setDate(QDateTime::fromString("2017-06-01", "yyyy-MM-dd").date());
    ui->endDateTimeEdit->setDate(QDateTime::fromString("2017-06-03", "yyyy-MM-dd").date());
    m_manager = new QNetworkAccessManager(this);
}

RESTDialog::~RESTDialog()
{
    delete ui;
}

void RESTDialog::on_genURLButton_clicked()
{
    char *formats[] = {"waterml,2.0", "rdb", "rdb,1.0", "json"};
    char buffer[32767];

    /*
    // Return most recent values only
    qsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]),
            "https://waterservices.usgs.gov/nwis/iv/?format=%s&sites=%s&parameterCd=00060,00065&siteStatus=all",
            formats[ui->formatComboBox->currentIndex()],
            ui->siteLineEdit->text().toStdString().c_str());
    */
    qsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]),
            //"https://waterservices.usgs.gov/nwis/iv/?format=%s&sites=%s&startDT=%s-0700&endDT=%s-0700&parameterCd=00060,00065&siteStatus=all",
            "https://waterservices.usgs.gov/nwis/iv/?format=%s&sites=%s&startDT=%s&endDT=%s&parameterCd=00060,00065&siteStatus=all",
            formats[ui->formatComboBox->currentIndex()],
            ui->siteLineEdit->text().toStdString().c_str(),
            ui->startDateTimeEdit->dateTime().toString(Qt::ISODate).toStdString().c_str(),
            ui->endDateTimeEdit->dateTime().toString(Qt::ISODate).toStdString().c_str()
            );
    qDebug() << buffer;

    //char surl[] = "https://nwis.waterservices.usgs.gov/nwis/iv/?format=rdb&sites=01646500&startDT=2017-06-01&endDT=2017-06-03&parameterCd=00060,00065&siteStatus=all";
    char *surl = "https://nwis.waterservices.usgs.gov/nwis/iv/?format=rdb&sites=01646500&startDT=2017-06-01&endDT=2017-06-03&parameterCd=00060,00065&siteStatus=all";
    qDebug() << surl;

    surl = buffer;

    //ui->urlEdit->setText(buffer);
    ui->urlEdit->setText(surl);
    ui->urlEdit->selectAll();

    //QUrl url(buffer);
    QUrl url(surl);

    QNetworkRequest request;
    request.setUrl(url);

    // check for redirect
    QNetworkReply *reply = m_manager->get(request);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    // QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    /**
    do {
        QNetworkRequest request;
        request.setUrl(url);

        QNetworkReply *reply = m_manager->get(request);

        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    } while (redirectUrl.toString().size() != 0);
    **/


    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    // if (redirectUrl.toString().size() != 0) {
    while (redirectUrl.toString().size() != 0) {
        Q_ASSERT(reply->readAll().size() == 0);
        request.setUrl(redirectUrl);
        reply = m_manager->get(request);
        /**
        connect(m_manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(replyFinished(QNetworkReply*)));
                **/
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    }
    ui->outputPlainTextEdit->document()->setPlainText(reply->readAll());


    /*
    m_isWaitingHttpResponse = true;
    connect(m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    QNetworkReply *reply = m_manager->get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));

    connect(reply, SIGNAL(finished()),
            this, SLOT(finished()));

    qDebug() << "reply:" << reply;
            */

    /*
    qApp->processEvents();
    while (m_isWaitingHttpResponse) {
        qApp->thread()->wait(100);
        qApp->processEvents();
    }
    */

}

void RESTDialog::finished()
{
    qDebug() << "RESTDialog::finished:";
    //m_isWaitingHttpResponse = false;
}

void RESTDialog::error(QNetworkReply::NetworkError code)
{
    qDebug() << "RESTDialog::error:" << code;
}

void RESTDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << "bytesReceived" << bytesReceived << " of " << bytesTotal;
}

void RESTDialog::replyFinished(QNetworkReply* reply)
{
    qDebug() << "RESTDialog::replyFinished";
    qDebug() << "reply:" << reply;
    if (reply->error() > 0) {
        qDebug() << "errors";
        ui->outputPlainTextEdit->document()->setPlainText(reply->errorString());
    }
    else {
        qDebug() << "no errors " << reply->errorString();
        //{{
        qDebug() << "Content-Type:" << reply->header(QNetworkRequest::ContentTypeHeader);
        qDebug() << "ContentLengthHeader:" << reply->header(QNetworkRequest::ContentLengthHeader);
        qDebug() << "Url:" << reply->url().url();
        qDebug() << "Redirect" << reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "rawHeaderList:" << reply->rawHeaderList();
        //}}
        qDebug() << "isRunning:" << reply->isRunning();
        qDebug() << "isFinished:" << reply->isFinished();
        qDebug() << "readBufferSize:" << reply->readBufferSize();
        qDebug() << "bytesAvailable:" << reply->bytesAvailable();
        ui->outputPlainTextEdit->document()->setPlainText(reply->readAll());
        //ui->outputPlainTextEdit->document()->setPlainText(reply->read(500));
    }
    reply->deleteLater();

    // after first get() doesn't work for some reason
    delete m_manager;
    m_manager = new QNetworkAccessManager(this);
    //m_isWaitingHttpResponse = false;
}

