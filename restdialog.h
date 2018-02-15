#ifndef RESTDIALOG_H
#define RESTDIALOG_H

#include <QWidget>
#include <QNetworkReply>

namespace Ui {
class RESTDialog;
}

class QNetworkAccessManager;
class QNetworkReply;

class RESTDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RESTDialog(QWidget *parent = 0);
    ~RESTDialog();

private slots:
    void on_genURLButton_clicked();
    void replyFinished(QNetworkReply*);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();

private:
    Ui::RESTDialog *ui;
    QNetworkAccessManager *m_manager;
    bool m_isWaitingHttpResponse;
};

#endif // RESTDIALOG_H
