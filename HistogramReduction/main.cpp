#include "mainwindow.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <fstream>
#include <cmath>

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

QImage bmp_to_image(const std::vector<unsigned char>& data, uint32_t width, uint32_t height)
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

QImage hist_to_image(const std::vector<unsigned char>& data)
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
    for(int i = 0; i < len; i++)
    {
        for(int j = 0; j < round((double)ptr[i]); j++)
        {
            image.setPixel(i, 256 - 1 - j, qRgb(0, 200, 0));
        }
    }
    return image;
}

void image_to_hist(const std::vector<unsigned char>& image, uint32_t hist[256])
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
    }
}

std::vector<unsigned char> change_hist(const std::vector<unsigned char>& original_image,
                                       uint32_t target_hist[256])
{
    std::vector<unsigned char> result_image(original_image.size());

    uint32_t oh[256];
    image_to_hist(original_image, oh);
    uint8_t od[256], td[256];
    hist_to_dist(oh, od);
    hist_to_dist(target_hist, td);

    uint8_t id[256];
    uint32_t prev = 0;
    for(int i = 0; i < 256; i++)
    {
        for(int j = prev; j < 256; j++)
        {
            if(td[j] - i >= 0)
            {
                id[i] = j;
                prev = j;
                break;
            }
        }
    }

    for(int i = 0; i < original_image.size(); i++)
    {
        uint8_t t = od[original_image[i]];
        result_image[i] = id[t];
    }
    return result_image;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::vector<unsigned char> original_data;
    uint32_t width, height;
    read_bmp("finger.bmp", original_data, width, height);

    std::vector<unsigned char> target_data;
    uint32_t target_width, target_height;
    read_bmp("target.bmp", target_data, target_width, target_height);

    uint32_t original_hist_data[256];
    unsigned char original_dist_data[256];
    image_to_hist(original_data, original_hist_data);
    hist_to_dist(original_hist_data, original_dist_data);

    uint32_t target_hist_data[256];
    unsigned char target_dist_data[256];
    image_to_hist(target_data, target_hist_data);
    hist_to_dist(target_hist_data, target_dist_data);

    std::vector<unsigned char> changed_data = change_hist(original_data, target_hist_data);

    uint32_t changed_hist_data[256];
    unsigned char changed_dist_data[256];
    image_to_hist(changed_data, changed_hist_data);
    hist_to_dist(changed_hist_data, changed_dist_data);

    QImage original_image = bmp_to_image(original_data, width, height);
    QImage target_image = bmp_to_image(target_data, target_width, target_height);
    QImage changed_image = bmp_to_image(changed_data, width, height);
    QImage original_hist = hist_to_image(original_data);
    QImage target_hist = hist_to_image(target_data);
    QImage changed_hist = hist_to_image(changed_data);
    QImage original_dist = data_to_image(original_dist_data, 256);
    QImage target_dist = data_to_image(target_dist_data, 256);
    QImage changed_dist = data_to_image(changed_dist_data, 256);


    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(original_image))->setPos(0, 0);
    scene.addPixmap(QPixmap::fromImage(target_image))->setPos(0, height);
    scene.addPixmap(QPixmap::fromImage(changed_image))->setPos(0, target_height + height);

    scene.addPixmap(QPixmap::fromImage(original_hist))->setPos(width, 0);
    scene.addPixmap(QPixmap::fromImage(target_hist))->setPos(target_width, height);
    scene.addPixmap(QPixmap::fromImage(changed_hist))->setPos(width, height + target_height);

    scene.addPixmap(QPixmap::fromImage(original_dist))->setPos(width + 256, 0);
    scene.addPixmap(QPixmap::fromImage(target_dist))->setPos(target_width + 256, height);
    scene.addPixmap(QPixmap::fromImage(changed_dist))->setPos(width + 256, height + target_height);

    QGraphicsView graphicsView(&scene);
    graphicsView.show();

    return a.exec();
}

