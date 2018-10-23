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

void MainWindow::paintEvent(QPaintEvent *)
{
//    unsigned char* color =  new unsigned char[256];
//    for (int i = 0; i < 256; i++)
//    {
//        color[i] = static_cast<unsigned char>(i);
//    }

    unsigned char* data =  new unsigned char[256];
    for (int i = 0; i < 256; i++)
    {
        data[i] = static_cast<unsigned char>(i);
    }

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
