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

void read_bmp(const string& fileName, vector<unsigned char>& out_data, uint32_t& out_width, uint32_t& out_height)
{
    const uint32_t headers_size = 54;
    uint8_t headers[headers_size];

    ifstream file(fileName, std::ios::binary);
    file.read((char*)headers, headers_size);

    uint32_t fileSize = *((uint32_t*)&headers[0x02]);
    uint32_t dataOffset = *((uint32_t*)&headers[0x0A]);
    int32_t width = *((int32_t*)&headers[0x12]);
    int32_t height = *((int32_t*)&headers[0x16]);
    uint32_t imageSize = *((uint32_t*)&headers[0x22]);

    out_width = abs(width);
    out_height = abs(height);

    if(imageSize == 0)
        imageSize = out_height * out_width;

    out_data.resize(imageSize);
    file.seekg(dataOffset);
    file.read((char*)out_data.data(), imageSize);
}

QImage bmp_to_image(const vector<unsigned char>& data, uint32_t width, uint32_t height)
{
    QImage image(width, height, QImage::Format_RGB888);

    for(uint32_t i = 0; i < width; i++)
    {
        for(uint32_t j = 0; j < height; j++)
        {
            unsigned char color = data[j * width + i];
            image.setPixel(i, j, qRgb(color, color, color));
        }
    }
    return image;
}

template<typename I>
Image<uint8_t> binary_image(const I& image, typename I::element_t threshold)
{
    Image<uint8_t> result(image.width(), image.height());
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

//uint8_t q2i_conv_func(const QImage& qImage, int32_t x, int32_t y)
//{
//    return static_cast<uint8_t>(qImage.pixelIndex(x, y));
//}


uint i2q_conv_func(const uint8_t& e)
{
    return qRgb(e, e, e);
}

QImage image_to_qImage(const Image<uint8_t>& image)
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


Image<uint8_t> qImage_to_image(const QImage& qImage)
{
    Image<uint8_t> image(qImage.width(), qImage.height());
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

    std::vector<unsigned char> original_data;
    uint32_t width, height;
    read_bmp("../F1_7.bmp", original_data, width, height);

    QImage original_image = bmp_to_image(original_data, width, height);

    Image<uint8_t> image = qImage_to_image(original_image);

    Image<uint8_t> binaryImage = binary_image(image, 120);
    QImage qBinaryImage = image_to_qImage(binaryImage);

    Image<uint8_t> skeletImage = skelet(binaryImage, [](uint8_t v) {return v == 255;}, 255);
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
