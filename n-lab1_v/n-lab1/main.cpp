#include "mainwindow.h"
#include "skeletwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
    SkeletWindow w;
    w.show();

    return a.exec();
}
