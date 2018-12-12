#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <fstream>
#include <cmath>
#include <complex>

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

QImage array_to_hist(const std::vector<unsigned char>& data, int col_width = 10, int image_height = 256)
{
    QImage result(data.size() * col_width, image_height, QImage::Format_RGB888);
    result.fill(qRgb(0, 0, 0));

    for(int i = 0; i < data.size(); i++)
    {
        for(int col = 0; col < col_width; col++)
        {
            for(int h = 0; h < data[i]; h++)
                result.setPixel(i * col_width + col, image_height - 1 - h, qRgb(255, 0, 0));
        }
    }

    return result;
}

void ft(const std::vector<std::complex<double>>& data, uint32_t width, uint32_t height, std::vector<std::complex<double>>& result)
{
    result.clear();
    result.resize(data.size());

    std::vector<std::complex<double>> t(data.size());

    for(int row = 0; row < height; row++)
    {
        for(int u = 0; u < width; u++)
        {
            std::complex<double> sum = 0;
            for(int x = 0; x < width; x++)
            {
                sum += data[row * width + x] *
                        std::complex<double>(cos((2.0 * M_PI * u * x) / width),
                                             -sin((2.0 * M_PI * u * x) / width));
            }
            t[row * width + u] = sum;
        }
    }

    for(int col = 0; col < width; col++)
    {
        for(int u = 0; u < height; u++)
        {
            std::complex<double> sum = 0;
            for(int y = 0; y < height; y++)
            {
                sum += t[y * width + col] *
                        std::complex<double>(cos((2.0 * M_PI * u * y) / height),
                                             -sin((2.0 * M_PI * u * y) / height));
            }
            result[u * width + col] = sum;
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::vector<unsigned char> original_data;
    uint32_t width, height;
    read_bmp("F99_6.bmp", original_data, width, height);

    std::vector<std::complex<double>> complex_data(original_data.size());
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
            complex_data[y * width + x] = original_data[y * width + x] * pow(-1, x + y);
    }

    std::vector<std::complex<double>> ft_result_data;
    ft(complex_data, width, height, ft_result_data);

    std::vector<double> energy(original_data.size());
    for(int i = 0; i < original_data.size(); i++)
        energy[i] = 0.5 * log(1 + sqrt(pow(ft_result_data[i].real(), 2) + pow(ft_result_data[i].imag(), 2)));

    double min_value = *std::min_element(energy.begin(), energy.end());
    for(int i = 0; i < energy.size(); i++)
        energy[i] -= min_value;

    double max_value = *std::max_element(energy.begin(), energy.end());
    std::vector<unsigned char> byte_energy(energy.size());
    for(int i = 0; i < energy.size(); i++)
        byte_energy[i] = energy[i] / max_value * 255;

    QImage original_image = bmp_to_image(original_data, width, height);
    QImage ft_image = bmp_to_image(byte_energy, width, height);


    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(original_image))->setPos(0, 0);
    scene.addPixmap(QPixmap::fromImage(ft_image))->setPos(original_image.width(), 0);
    //scene.addPixmap(QPixmap::fromImage(ft_hist_image))->setPos(original_image.width(), slice_hist_image.height());

    QGraphicsView graphicsView(&scene);
    graphicsView.show();

    return a.exec();
}











