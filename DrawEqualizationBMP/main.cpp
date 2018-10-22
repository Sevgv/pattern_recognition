#include "mainwindow.h"
#include "bmpstruct.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    FILE *pFile;
    errno_t e = fopen_s(&pFile, "F1_7.bmp", "rb");
    if (e)
    {
        w.Message();
        return NULL;
    }
    w.fBMP = new FileBMP;
    w.fBMP->init(pFile);

    w.resize(w.fBMP->get_Width(), w.fBMP->get_Height());
    w.show();
    w.update();

    return a.exec();
}
