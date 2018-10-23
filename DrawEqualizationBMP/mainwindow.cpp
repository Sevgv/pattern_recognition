#include "mainwindow.h"
#include "bmpstruct.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QPainter>
#include <qmath.h>

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
