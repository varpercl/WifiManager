#include "wifimanager.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WifiManager w;
    w.show();

    return a.exec();
}
