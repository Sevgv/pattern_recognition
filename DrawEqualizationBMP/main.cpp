#include "mainwindow.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QAction>


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
    w.move(0, 0);
    w.show();

//    QImage img = w.paint_image(w.fBMP->arrIndexes, w.fBMP->get_Width(), w.fBMP->get_Height());
//    QImage bar_chart = w.paint_bar_chart(w.fBMP->arrIndexes, w.fBMP->get_Width(), w.fBMP->get_Height());

//    QGraphicsScene scene_img;
//    scene_img.addPixmap(QPixmap::fromImage(img))->setPos(0, 0);

//    QGraphicsScene scene_bar_chart;
//    scene_bar_chart.addPixmap(QPixmap::fromImage(bar_chart))->setPos(0, 0);

//    QGraphicsView graphicsView_scene_img(&scene_img);
//    QGraphicsView graphicsView_scene_bar_chart(&scene_bar_chart);

//    graphicsView_scene_img.move(w.fBMP->get_Width() + 16, 0);
//    graphicsView_scene_img.show();

//    graphicsView_scene_bar_chart.move(0, w.fBMP->get_Height() + 38);
//    graphicsView_scene_bar_chart.show();

    return a.exec();
}
