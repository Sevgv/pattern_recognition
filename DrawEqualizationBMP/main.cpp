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
//    errno_t e = fopen_s(&pFile, "F1_7.bmp", "rb");
        errno_t e = fopen_s(&pFile, "F00027_21.bmp", "rb");
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

    QAction* exitAction=new QAction("&Exit", qApp);
    exitAction->setShortcut(Qt::Key_Escape);
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

//    QImage img = w.paint_image(w.fBMP->arrIndexes, w.fBMP->get_Width(), w.fBMP->get_Height());
    QImage eq_img = w.paint_equliz_image(w.fBMP->arrIndexes, w.fBMP->get_Width(), w.fBMP->get_Height());
    QImage hist = w.paint_hist(w.fBMP->arrIndexes, w.fBMP->get_Width(), w.fBMP->get_Height());
    QImage eq_hist = w.paint_equliz_hist(w.fBMP->arrIndexes, w.fBMP->get_Width(), w.fBMP->get_Height());

//    QGraphicsScene scene_img;
//    scene_img.addPixmap(QPixmap::fromImage(img))->setPos(0, 0);

    QGraphicsScene scene_eq_img;
    scene_eq_img.addPixmap(QPixmap::fromImage(eq_img))->setPos(0, 0);

    QGraphicsScene scene_hist;
    scene_hist.addPixmap(QPixmap::fromImage(hist))->setPos(0, 0);

    QGraphicsScene scene_eq_hist;
    scene_eq_hist.addPixmap(QPixmap::fromImage(eq_hist))->setPos(0, 0);



//    QGraphicsView graphicsView_scene_img(&scene_img);
//    graphicsView_scene_img.addAction(exitAction);

    QGraphicsView graphicsView_scene_eq_img(&scene_eq_img);
    graphicsView_scene_eq_img.addAction(exitAction);

    QGraphicsView graphicsView_scene_hist(&scene_hist);
    graphicsView_scene_hist.addAction(exitAction);

    QGraphicsView graphicsView_scene_eq_hist(&scene_eq_hist);
    graphicsView_scene_eq_hist.addAction(exitAction);


//    graphicsView_scene_img.move(w.fBMP->get_Width() + 16, 0);
//    graphicsView_scene_img.show();

    graphicsView_scene_eq_img.move(w.fBMP->get_Width() + 16, 0);
    graphicsView_scene_eq_img.show();

    graphicsView_scene_hist.move(0, w.fBMP->get_Height() + 38);
    graphicsView_scene_hist.show();

    graphicsView_scene_eq_hist.move(w.fBMP->get_Width() + 16, w.fBMP->get_Height() + 38);
    graphicsView_scene_eq_hist.show();




    return a.exec();
}
