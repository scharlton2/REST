#include "restdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RESTDialog w;
    w.show();

    return a.exec();
}
