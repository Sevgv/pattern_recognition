#include "mainwindow.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <fstream>
#include <cmath>
#include <QAction>

using namespace std;

void read_bmp(const string& fileName, vector<unsigned char>& out_data, uint32_t& out_width, uint32_t& out_height)
{
    const uint32_t headers_size = 54;
    uint8_t headers[headers_size];

    ifstream file(fileName, ios::binary);
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

QImage hist_to_image(const vector<unsigned char>& data)
{
    uint32_t hist[256];
    uint32_t max_value = 0;

    for(int i = 0; i < 256; i++)
    {
        hist[i] = 0;
    }

    for(int i = 0; i < data.size(); i++)
        hist[data[i]]++;

    for(int i = 0; i < 256; i++)
        if(hist[i] > max_value)
            max_value = hist[i];

    QImage image(256, 256, QImage::Format_RGB888);
    image.fill(qRgb(0, 0, 0));
    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < round((double)hist[i] / ((double)max_value / 256.0)); j++)
        {
            image.setPixel(i, 256 - 1 - j, qRgb(255, 0, 0));
        }
    }

    return image;
}

QImage data_to_image(unsigned char* ptr, int len)
{
    unsigned char max_value = 0;
    for(int i = 0; i < len; i++)
        if(ptr[i] > max_value)
            max_value = ptr[i];

    QImage image(len, max_value + 1, QImage::Format_RGB888);
    image.fill(qRgb(0, 0, 0));
    for(int i = 0; i < len; i++)
    {
        for(int j = 0; j < round((double)ptr[i]); j++)
        {
            image.setPixel(i, 256 - 1 - j, qRgb(0, 200, 100));
        }
    }
    return image;
}

void image_to_hist(const vector<unsigned char>& image, uint32_t hist[256])
{
    for(int i = 0; i < 256; i++)
    {
        hist[i] = 0;
    }

    for(int i = 0; i < image.size(); i++)
        hist[image[i]]++;
}

void hist_to_dist(uint32_t hist[256], uint8_t dist[256])
{
    uint32_t total = 0;
    uint32_t sum = 0;
    for(int i = 0; i < 256; i++)
        total += hist[i];

    for(int i = 0; i < 256; i++)
    {
        sum += hist[i];
        dist[i] = round((double)sum / (double)total * 255.0);
        uint8_t a = dist[i];
    }
}

unsigned char average_value(const vector<unsigned char>& original_image, int index, int size, uint32_t width)
{
    int value = 0;

    for(int i = 0; i < size*width; i+=width)
    {
        for(int j = 0; j < size; j++)
        {
            value += original_image[i+j+index];
        }
    }

    value = round((double)(value/(size*size)));

    return (unsigned char)value;
}


void image_to_smoothed(const vector<unsigned char>& original_image,
                       vector<unsigned char>& smoothed_data,
                       uint32_t width, int size)
{
    smoothed_data.resize(original_image.size());

    for(int i = 0; i < original_image.size(); i++)
    {
//        if(i+3 <= original_image.size())
//        {
//            smoothed_data[i] = round((double)(original_image[i] + original_image[i+1] + original_image[i+2] +
//                                original_image[i+width]+ original_image[i+width+1] + original_image[i+width+2] +
//                                original_image[i+(2*width)] + original_image[i+(2*width)+1] + original_image[i+(2*width)+2])/9);
        smoothed_data[i] = average_value(original_image, i, size, width);
//        }
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QAction* exitAction=new QAction("&Exit", qApp);
    exitAction->setShortcut(Qt::Key_Escape);
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    vector<unsigned char> original_data;
    uint32_t width, height;
    read_bmp("F99_6.bmp", original_data, width, height);
//    read_bmp("F00022_06.bmp", original_data, width, height);
//    read_bmp("F00027_21.bmp", original_data, width, height);

    uint32_t original_hist_data[256];
    unsigned char original_dist_data[256];
    image_to_hist(original_data, original_hist_data);
    hist_to_dist(original_hist_data, original_dist_data);

    QImage original_image = bmp_to_image(original_data, width, height);
    QImage original_hist = hist_to_image(original_data);
    QImage original_dist = data_to_image(original_dist_data, 256);

    vector<unsigned char> smoothed_data;
    image_to_smoothed(original_data, smoothed_data, width, 3);

    uint32_t smoothed_hist_data[256];
    unsigned char smoothed_dist_data[256];
    image_to_hist(smoothed_data, smoothed_hist_data);
    hist_to_dist(smoothed_hist_data, smoothed_dist_data);
    QImage smoothed_image = bmp_to_image(smoothed_data, width, height);
    QImage smoothed_hist = hist_to_image(smoothed_data);
    QImage smoothed_dist = data_to_image(smoothed_dist_data, 256);


    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(original_image))->setPos(0, 0);
    scene.addPixmap(QPixmap::fromImage(original_hist))->setPos(0, height);
    scene.addPixmap(QPixmap::fromImage(original_dist))->setPos(256, height);

    bool flag = true;
    if(width >= 512)
    {
        scene.addPixmap(QPixmap::fromImage(smoothed_image))->setPos(width, 0);
        scene.addPixmap(QPixmap::fromImage(smoothed_hist))->setPos(width, height);
        scene.addPixmap(QPixmap::fromImage(smoothed_dist))->setPos(width + 256, height);
    }
    else
    {
        scene.addPixmap(QPixmap::fromImage(smoothed_image))->setPos(512, 0);
        scene.addPixmap(QPixmap::fromImage(smoothed_hist))->setPos(512, height);
        scene.addPixmap(QPixmap::fromImage(smoothed_dist))->setPos(512+256, height);
        flag = false;
    }

    QGraphicsView graphicsView(&scene);
    graphicsView.addAction(exitAction);
    if (!flag)
        graphicsView.setGeometry(QRect(0, 0, width*2, height+256));
    else
        graphicsView.setGeometry(QRect(0, 0, 1024, height+256));

    graphicsView.show();

//    MainWindow w;
//    w.show();

    return a.exec();
}
