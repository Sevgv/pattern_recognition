#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    Draw();
}

MainWindow::~MainWindow()
{
    delete ui;
}
//void MainWindow::Draw()
//{
////    QString FileName = QFileDialog::getOpenFileName(this, "title", "F1_7.bmp");//, "Bitmap Images(*.bmp)" "Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
////    image = QImage(FileName);
//    // QLayout::setContentsMargins(0, 0, 0, 0);  //????????
//    QGraphicsScene *scene = new QGraphicsScene(this);
//    QGraphicsView *view = new QGraphicsView(scene);

//    unsigned char* color =  new unsigned char[256];
//    for (int i = 0; i < 256; i++)
//    {
//        color[i] = static_cast<unsigned char>(i);
//    }

//    QPainter painter;
//     painter.begin(this);
//     QImage img(fBMP->get_Width(),fBMP->get_Height(),QImage::Format_Indexed8);
//     for (int i = 0; i < fBMP->get_Height(); i++) {
//         for (int j = 0; j < fBMP->get_Width(); j++) {
//       img.setPixel(j, i, color[fBMP->arrIndexes[i][j]]);
//      }
//     }
//     painter.drawImage(0,0,img);
//     painter.end();

////    scene->addPixmap(QPixmap::fromImage(img));
//    // ui.graphicsView->setVisible(false);
//    // ui.graphicsView->setScene(scene);
//    // ui.graphicsView->show();
//    view->show();
//}

QImage MainWindow::paint_image(unsigned char** data, int width, int height)
{
    QImage image(width, height, QImage::Format_RGB888);

    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            unsigned char color = data[i][j];
            image.setPixel(j, i, qRgb(color, color, color));
        }
    }
    return image;
}

QImage MainWindow::paint_bar_chart(unsigned char** data, int width, int height)
{
    uint hist[256];
    uint max_value = 0;

    for(int i = 0; i < 256; i++)
    {
        hist[i] = 0;
    }

    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            hist[data[i][j]]++;            
        }
    }

    for(int i = 0; i < 256; i++)
        if(hist[i] > max_value)
            max_value = hist[i];

    QImage image(1024, 256, QImage::Format_RGB888);
    image.fill(qRgb(255, 255, 255));
    for(int i = 0; i < 1024; i+=4)
    {
        for(int j = 0; j < round((double)hist[i/4] / ((double)max_value / 256.0)); j++)
        {
            image.setPixel(i, 256 - 1 - j, qRgb(0, 0, 255));
            image.setPixel(i+1, 256 - 1 - j, qRgb(0, 0, 255));
            image.setPixel(i+2, 256 - 1 - j, qRgb(0, 0, 255));
            image.setPixel(i+3, 256 - 1 - j, qRgb(0, 0, 255));
        }
    }

    return image;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView *graphicsView_scene_img = new QGraphicsView;
    QGraphicsView *graphicsView_scene_bar_chart = new QGraphicsView;

    int key = event->key();
    if (key == Qt::Key_Escape) {
        QApplication::quit();
     }
    if (key == Qt::Key_Space) {

        QAction* exitAction=new QAction(tr("Exit"), qApp);
        exitAction->setShortcut(Qt::Key_Escape);
        connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

        QImage *img = new QImage(paint_image(fBMP->arrIndexes, fBMP->get_Width(), fBMP->get_Height()));
        QImage *bar_chart = new QImage(paint_bar_chart(fBMP->arrIndexes, fBMP->get_Width(), fBMP->get_Height()));

        QGraphicsScene *scene_img = new QGraphicsScene();
        scene_img->addPixmap(QPixmap::fromImage(*img))->setPos(0, 0);

        QGraphicsScene *scene_bar_chart = new QGraphicsScene();
        scene_bar_chart->addPixmap(QPixmap::fromImage(*bar_chart))->setPos(0, 0);

        graphicsView_scene_img = new QGraphicsView(scene_img);
        graphicsView_scene_img->addAction(exitAction);

        graphicsView_scene_bar_chart = new QGraphicsView(scene_bar_chart);
        graphicsView_scene_bar_chart->addAction(exitAction);

        graphicsView_scene_img->move(fBMP->get_Width() + 16, 0);
        graphicsView_scene_img->show();


        graphicsView_scene_bar_chart->move(0, fBMP->get_Height() + 38);
        graphicsView_scene_bar_chart->show();

     }


}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);
    QImage img(fBMP->get_Width(), fBMP->get_Height(), QImage::Format_RGB888);
    for (int i = 0; i < fBMP->get_Height(); i++) {
        for (int j = 0; j < fBMP->get_Width(); j++) {
            unsigned char color = fBMP->arrIndexes[i][j];
            img.setPixel(j, i, qRgb(color, color, color));
      }
     }
     painter.drawImage(0,0,img);
     painter.end();
}

void MainWindow::Message()
{
    QMessageBox::information(this,
        "Ошибка",
        "Файл не найден!");
}
