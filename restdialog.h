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

private:
    Ui::RESTDialog *ui;
    QNetworkAccessManager *m_manager;
};

#endif // RESTDIALOG_H
