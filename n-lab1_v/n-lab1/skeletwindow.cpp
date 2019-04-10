#include "skeletwindow.h"
#include "ui_skeletwindow.h"
#include "image_qt.hpp"
#include "operations.hpp"
#include "skelet.hpp"

SkeletWindow::SkeletWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SkeletWindow)
{
    ui->setupUi(this);

    ui->originalImage->setScene(new QGraphicsScene(this));
    ui->binaryImage->setScene(new QGraphicsScene(this));
    ui->skeletImage->setScene(new QGraphicsScene(this));

    QImage qImage("test.bmp");
    Image<uint8_t> image = qImage_to_image<uint8_t>(qImage, q2i_conv_func_color);

    ui->originalImage->scene()->addPixmap(QPixmap::fromImage(qImage));

    Image<uint8_t> binaryImage = binary_image(image, 200);
    QImage qBinaryImage = image_to_qImage(binaryImage);

    ui->binaryImage->scene()->addPixmap(QPixmap::fromImage(qBinaryImage));

    Image<uint8_t> skeletImage = skelet(binaryImage, [](uint8_t v) {return v == 255;}, 255);
    QImage qSkeletImage = image_to_qImage(skeletImage);

    ui->skeletImage->scene()->addPixmap(QPixmap::fromImage(qSkeletImage));
}

SkeletWindow::~SkeletWindow()
{
    delete ui;
}
