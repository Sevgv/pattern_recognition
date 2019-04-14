#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QAction>
#include <iostream>
#include <fstream>
#include <cmath>
#include "skelet.hpp"

using namespace std;

template<typename I>
Image binary_image(const I& image, uint8_t threshold)
{
    Image result(image.width(), image.height());
    for(int32_t i = 0; i < image.size(); i++)
    {
        result.element(i) = image.element(i) > threshold ? 255 : 0;
    }
    return result;
}

uint8_t q2i_conv_func_color(const QImage& qImage, int32_t x, int32_t y)
{
    return static_cast<uint8_t>(qImage.pixelColor(x, y).red());
}

uint i2q_conv_func(const uint8_t& e)
{
    return qRgb(e, e, e);
}

QImage image_to_qImage(const Image& image)
{
    QImage::Format format = QImage::Format_RGB888;
    QImage qImage(image.width(), image.height(), format);
    for(int32_t x = 0; x < image.width(); x++)
    {
        for(int32_t y = 0; y < image.height(); y++)
        {
            qImage.setPixel(x, y, i2q_conv_func(image.element(x, y)));
        }
    }
    return qImage;
}


Image qImage_to_image(const QImage& qImage)
{
    Image image(qImage.width(), qImage.height());
    for(int32_t x = 0; x < qImage.width(); x++)
        for(int32_t y = 0; y < qImage.height(); y++)
            image.element(x, y) = q2i_conv_func_color(qImage, x, y);
    return image;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QAction* exitAction=new QAction("&Exit", qApp);
    exitAction->setShortcut(Qt::Key_Escape);
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QImage original_image("../F1_7.bmp");

    Image image = qImage_to_image(original_image);

    Image binaryImage = binary_image(image, 120);
    QImage qBinaryImage = image_to_qImage(binaryImage);

    Image skeletImage = skelet(binaryImage, [](uint8_t v) {return v == 255;}, 255);
    QImage qSkeletImage = image_to_qImage(skeletImage);

    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(original_image))->setPos(0, 0);
    scene.addPixmap(QPixmap::fromImage(qBinaryImage))->setPos(original_image.width(), 0);
    scene.addPixmap(QPixmap::fromImage(qSkeletImage))->setPos(original_image.width() + qBinaryImage.width(), 0);

    QGraphicsView graphicsView(&scene);
    graphicsView.addAction(exitAction);
    graphicsView.showMaximized();

    return a.exec();
}
