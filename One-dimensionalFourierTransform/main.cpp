#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <fstream>
#include <cmath>
#include <complex>
#include <QAction>

using namespace std;

void read_bmp(const string& fileName, vector<unsigned char>& out_data, uint32_t& out_width, uint32_t& out_height)
{
    const uint32_t headers_size = 54;
    uint8_t headers[headers_size];

    ifstream file(fileName, std::ios::binary);
    file.read(reinterpret_cast<char*>(headers), headers_size);

    uint32_t fileSize = *(reinterpret_cast<uint32_t*>(&headers[0x02]));
    uint32_t dataOffset = *(reinterpret_cast<uint32_t*>(&headers[0x0A]));
    int32_t width = *(reinterpret_cast<int32_t*>(&headers[0x12]));
    int32_t height = *(reinterpret_cast<int32_t*>(&headers[0x16]));
    uint32_t imageSize = *(reinterpret_cast<uint32_t*>(&headers[0x22]));

    out_width = static_cast<uint32_t>(abs(width));
    out_height = static_cast<uint32_t>(abs(height));

    if(imageSize == 0)
        imageSize = out_height * out_width;

    out_data.resize(imageSize);
    file.seekg(dataOffset);
    file.read(reinterpret_cast<char*>(out_data.data()), imageSize);
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

void ft(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result)
{
    result.clear();
    result.resize(data.size());

    for(int u = 0; u < data.size(); u++)
    {
        std::complex<double> sum = 0;
        for(int x = 0; x < data.size(); x++)
        {
            sum += data[x] * pow(-1,x)* std::complex<double>(cos((2.0 * M_PI * u * x) / data.size()), -sin((2.0 * M_PI * u * x) / data.size()));
        }
        result[u] = sum /static_cast<double>(data.size());
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QAction* exitAction=new QAction("&Exit", qApp);
    exitAction->setShortcut(Qt::Key_Escape);
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    std::vector<unsigned char> original_data;
    uint32_t width, height;
    read_bmp("F1_7.bmp", original_data, width, height);

    std::vector<std::complex<double>> slice_data(32);
    std::vector<unsigned char> slice_data_real(slice_data.size());
    for(int i = 0; i < 32; i++)
    {
        int y = 200;
        unsigned char p = original_data[y*width+i+200];
        slice_data[i] = std::complex<double>(p);
        slice_data_real[i] = p;
    }

    std::vector<std::complex<double>> ft_result_data;
    ft(slice_data, ft_result_data);

    QImage original_image = bmp_to_image(original_data, width, height);
    QImage slice_hist_image = array_to_hist(slice_data_real);

    std::vector<double> ft_en(ft_result_data.size());
    for(int i = 0; i < ft_result_data.size(); i++)
        ft_en[i] = pow(std::real(ft_result_data[i]), 2) + pow(std::imag(ft_result_data[i]), 2);

    ft_en[0] = 0;

    double max_value = std::numeric_limits<double>::min();
    for(int i = 0; i < ft_en.size(); i++)
        if(ft_en[i] > max_value)
            max_value = ft_en[i];

    for(int i = 0; i < ft_en.size(); i++)
        slice_data_real[i] = static_cast<unsigned char>(ft_en[i] / max_value * 255);

    QImage ft_hist_image = array_to_hist(slice_data_real);

    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(original_image))->setPos(0, 0);
    scene.addPixmap(QPixmap::fromImage(slice_hist_image))->setPos(original_image.width(), 0);
    scene.addPixmap(QPixmap::fromImage(ft_hist_image))->setPos(original_image.width(), slice_hist_image.height());

    QGraphicsView graphicsView(&scene);
    graphicsView.addAction(exitAction);
    graphicsView.show();

    return a.exec();
}

