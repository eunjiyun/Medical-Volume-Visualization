#include "ViewerSample.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ViewerSample w;
    w.show();
    return a.exec();
}
