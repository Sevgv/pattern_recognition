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

void imhist(unsigned char** data, int width, int height, uint hist[])
{

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

}

void cumhist(uint histogram[], uint cumhistogram[])
{
    cumhistogram[0] = histogram[0];

    for(int i = 1; i < 256; i++)
    {
        cumhistogram[i] = histogram[i] + cumhistogram[i-1];
    }
}

QImage MainWindow::paint_hist(unsigned char** data, int width, int height)
{
    uint hist[256];
    uint max_value = 0;

    imhist(data, width, height, hist);

    for(int i = 0; i < 256; i++)
        if(hist[i] > max_value)
            max_value = hist[i];

    int hist_w = 512; int hist_h = 400;

    QImage image(hist_w, hist_h, QImage::Format_RGB888);
    int bin_w = round((double) hist_w/256);
    image.fill(qRgb(255, 255, 255));
    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < round((double)hist[i] / ((double)max_value / (double)hist_h)); j++)
        {
            image.setPixel(bin_w*(i), hist_h - 1 - j, qRgb(0, 0, 255));
//            image.setPixel(i+1, 256 - 1 - j, qRgb(0, 0, 255));
//            image.setPixel(i+2, 256 - 1 - j, qRgb(0, 0, 255));
//            image.setPixel(i+3, 256 - 1 - j, qRgb(0, 0, 255));
        }
    }

    return image;
}

QImage MainWindow::paint_equliz_hist(unsigned char** data, int width, int height)
{
    uint hist[256];
    uint max_value = 0;

    imhist(data, width, height, hist);

    uint size = fBMP->get_SizeImage();
    double alpha = 255.0/size;

    // Calculate the probability of each intensity
    double PrRk[256];
    for(int i = 0; i < 256; i++)
    {
        PrRk[i] = (double)hist[i] / size;
    }

    uint cumhistogram[256];
    cumhist(hist, cumhistogram);

    // Scale the histogram
    int Sk[256];
    for(int i = 0; i < 256; i++)
    {
        Sk[i] = round((double)cumhistogram[i] * alpha);
    }

    // Generate the equlized histogram
    double PsSk[256];
    for(int i = 0; i < 256; i++)
    {
        PsSk[i] = 0;
    }

    for(int i = 0; i < 256; i++)
    {
        PsSk[Sk[i]] += PrRk[i];
    }

    uint final[256];
    for(int i = 0; i < 256; i++)
        final[i] = static_cast<uint>(round(PsSk[i]*255));

    for(int i = 0; i < 256; i++)
        if(final[i] > max_value)
            max_value = final[i];

    int hist_w = 512; int hist_h = 400;

    QImage image(hist_w, hist_h, QImage::Format_RGB888);
    int bin_w = round((double)hist_w/256);
    image.fill(qRgb(255, 255, 255));
    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < round((double)final[i] / ((double)max_value / (double)hist_h)); j++)
        {
            image.setPixel(bin_w*(i), hist_h - 1 - j, qRgb(0, 0, 255));
//            image.setPixel(i+1, 256 - 1 - j, qRgb(0, 0, 255));
//            image.setPixel(i+2, 256 - 1 - j, qRgb(0, 0, 255));
//            image.setPixel(i+3, 256 - 1 - j, qRgb(0, 0, 255));
        }
    }

    return image;
}

QImage MainWindow::paint_equliz_image(unsigned char** data, int width, int height)
{
    uint hist[256];
    uint max_value = 0;

    imhist(data, width, height, hist);

    uint size = fBMP->get_SizeImage();
    float alpha = 255.0/size;

    // Calculate the probability of each intensity
    float PrRk[256];
    for(int i = 0; i < 256; i++)
    {
        PrRk[i] = (double)hist[i] / size;
    }

    uint cumhistogram[256];
    cumhist(hist, cumhistogram);

    // Scale the histogram
    int Sk[256];
    for(int i = 0; i < 256; i++)
    {
        Sk[i] = round((double)cumhistogram[i] * alpha);
    }

    // Generate the equlized histogram
    float PsSk[256];
    for(int i = 0; i < 256; i++)
    {
        PsSk[i] = 0;
    }

    for(int i = 0; i < 256; i++)
    {
        PsSk[Sk[i]] += PrRk[i];
    }

    int final[256];
    for(int i = 0; i < 256; i++)
        final[i] = round(PsSk[i]*255);

    QImage image(width, height, QImage::Format_RGB888);
    for(int i = 0; i < fBMP->get_Height(); i++)
        for(int j = 0; j < fBMP->get_Width(); j++)
        {
            unsigned char color = static_cast<unsigned char>(Sk[data[i][j]]);
            image.setPixel(j, i, qRgb(color, color, color));
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
//    if (key == Qt::Key_Space) {

//        QAction* exitAction=new QAction(tr("Exit"), qApp);
//        exitAction->setShortcut(Qt::Key_Escape);
//        connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

//        QImage *img = new QImage(paint_image(fBMP->arrIndexes, fBMP->get_Width(), fBMP->get_Height()));
//        QImage *bar_chart = new QImage(paint_bar_chart(fBMP->arrIndexes, fBMP->get_Width(), fBMP->get_Height()));

//        QGraphicsScene *scene_img = new QGraphicsScene();
//        scene_img->addPixmap(QPixmap::fromImage(*img))->setPos(0, 0);

//        QGraphicsScene *scene_bar_chart = new QGraphicsScene();
//        scene_bar_chart->addPixmap(QPixmap::fromImage(*bar_chart))->setPos(0, 0);

//        graphicsView_scene_img = new QGraphicsView(scene_img);
//        graphicsView_scene_img->addAction(exitAction);

//        graphicsView_scene_bar_chart = new QGraphicsView(scene_bar_chart);
//        graphicsView_scene_bar_chart->addAction(exitAction);

//        graphicsView_scene_img->move(fBMP->get_Width() + 16, 0);
//        graphicsView_scene_img->show();


//        graphicsView_scene_bar_chart->move(0, fBMP->get_Height() + 38);
//        graphicsView_scene_bar_chart->show();

//     }


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
