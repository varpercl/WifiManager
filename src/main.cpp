#include "utama.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Utama w;
    w.show();

    return a.exec();
}
