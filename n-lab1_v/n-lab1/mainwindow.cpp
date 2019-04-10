#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "image_qt.hpp"
#include "grad.hpp"
#include "laplacian.hpp"
#include <iostream>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include "directional_field.hpp"
#include "math.h"
#include "operations.hpp"
#include "fourier_transform.hpp"

Image<double> rot_gaussian_hpf(int32_t width, int32_t height, double sigma2, double rot = 0.5)
{
    Image<double> result(width, height);
    int32_t center_u = width / 2, center_v = height / 2;
    for(int32_t u = 0; u < result.width(); u++)
    {
        for(int32_t v = 0; v < result.height(); v++)
        {
            double x = u - center_u;
            double y = v - center_v;

            double a = (x + y) * 10;
            double b = y * 0.1;

            double coord = -(pow(a, 2.0) + pow(b, 2.0));
            result.element(u, v) = 1.0 - exp(coord / (2.0 * sigma2))    + exp(-(pow(x, 2.0) + pow(y, 2.0)) / (2.0 * 4));
        }
    }
    return result;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->originalImage->setScene(new QGraphicsScene(this));
    ui->gradientFieldImage->setScene(new QGraphicsScene(this));
    ui->directionFieldImage->setScene(new QGraphicsScene(this));

    /*auto gauss = rot_gaussian_hpf(288, 477, 1000);
    QImage gauss_qImage = image_to_qImage(to_gray_image(gauss));
    ui->gradientFieldImage->scene()->addPixmap(QPixmap::fromImage(gauss_qImage));*/

    on_actionOpen_image_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

template<typename E, typename P>
void draw_line(Image<E, P>& image, int32_t x0, int32_t y0, int32_t x1, int32_t y1, E line_fill)
{
    const bool steep = (std::abs(y1 - y0) > std::abs(x1 - x0));
    if(steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if(x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const double dx = x1 - x0;
    const double dy = std::abs(y1 - y0);

    double error = dx / 2.0;
    const int32_t ystep = (y0 < y1) ? 1 : -1;
    int32_t y = y0;

    const int32_t maxX = x1;

    for(int32_t x = x0; x < maxX; x++)
    {
        if(steep)
        {
            if(!image.is_index_out_of_bounds(y, x))
                image.element(y, x) = line_fill;
        }
        else
        {
            if(!image.is_index_out_of_bounds(x, y))
                image.element(x, y) = line_fill;
        }

        error -= dy;
        if(error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

template<typename IP0, typename IP1, typename IP2>
Image<uint8_t> draw_direcional_field_on_image(const Image<uint8_t, IP0>& image, const Image<double, IP1>& direction_data, const Image<double, IP2>& strength_data, int32_t k_scale, int32_t dp = 20)
{
    double max_strength = 0;
    for(int32_t i = 0; i < strength_data.size(); i++)
        if(strength_data.element(i) > max_strength)
            max_strength = strength_data.element(i);

    Image<uint8_t> result = scale(image, k_scale);

    for(int32_t x = 0; x < image.width(); x += dp)
        for(int32_t y = 0; y < image.height(); y += dp)
        {
            int32_t center_x = k_scale * x + k_scale / 2, center_y = k_scale * y + k_scale / 2;
            double r = direction_data.element(x, y);
            double len = (k_scale * 0.7) * strength_data.element(x, y) / max_strength;
            draw_line<uint8_t>(result, center_x, center_y,
                               static_cast<int32_t>(round(center_x + len * cos(r))),
                               static_cast<int32_t>(round(center_y + len * sin(r))),
                               0);
        }

    return result;
}

template<typename IP0, typename IP1, typename IP2>
void draw_directional_field(QGraphicsScene* scene, const Image<uint8_t, IP0>& original_image, const Image<double, IP1>& direction_data, const Image<double, IP2>& strength_data, int32_t scale, int32_t dp = 20)
{
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(image_to_qImage(original_image)))->setScale(scale);

    double max_strength = 0;
    for(int32_t i = 0; i < strength_data.size(); i++)
        if(strength_data.element(i) > max_strength)
            max_strength = strength_data.element(i);

    const int32_t len = static_cast<int32_t>(scale * dp * 0.7);
    QPen pen(QBrush(QColor(255, 0, 0)), 1.0);

    for(int32_t x = 0; x < original_image.width(); x += dp)
    {
        for(int32_t y = 0; y < original_image.height(); y += dp)
        {
            double r = direction_data.element(x, y);
            double s = strength_data.element(x, y);

            QLineF l(0, 0, len * cos(r) * (s / max_strength), len * sin(r) * (s / max_strength));
            auto line = scene->addLine(l, pen);
            line->setPos(x * scale + scale / 2, y * scale + scale / 2);
        }
    }
}

template<typename K, typename I>
struct AvgDFFilter
{
    typedef DirectionalFieldElement new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        double direction = 0;
        double center_direction = image.element(x, y).direction;
        //std::cout << "center direction: " << center_direction << std::endl;
        for(auto e = kernel.elements().begin(); e != kernel.elements().end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
                continue;

            const DirectionalFieldElement& v = image.element(kx, ky);
            //double alter_direction = center_direction > 0 ? v.direction - M_PI : v.direction + M_PI;
            double alter_direction = 0;
            if(center_direction > 0)
            {
                alter_direction = v.direction + M_PI;
                //std::cout << "c: " << center_direction << "; d: " << v.direction << " +PI a: " << alter_direction << std::endl;
            }
            else
            {
                alter_direction = v.direction - M_PI;
                //std::cout << "c: " << center_direction << "; d: " << v.direction << " -PI a: " << alter_direction << std::endl;
            }

            //std::cout << "(c - d): " << abs(center_direction - v.direction) << "; (c - a): " << abs(center_direction - alter_direction) << std::endl;
            //std::cout << "cd: " << center_direction << "; d: " << v.direction << "; ad: " << alter_direction << std::endl;

            if(abs(center_direction - v.direction) < abs(center_direction - alter_direction))
                direction += v.direction;
            else
            {
                std::cout << "cd: " << center_direction << "; d: " << v.direction << "; ad: " << alter_direction << std::endl;
                direction += alter_direction;
            }
        }

        DirectionalFieldElement e = image.element(x, y);
        e.direction = direction / kernel.norm();
        return e;
    }
};

template<typename K, typename I>
struct TestFilter
{
    typedef DirectionalFieldElement new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        DirectionalFieldElement e = image.element(x, y);
        e.coherence = sqrt(e.Gx);
        return e;
    }
};

template<typename I>
Image<DirectionalFieldElement> average_directional_field(I& image, uint32_t kernel_size = 5)
{
    auto direction = image.template view_as<double>(directionalFieldViewDirection);
    double max_direction = std::numeric_limits<double>::min();
    double min_direction = std::numeric_limits<double>::max();
    for(int32_t i = 0; i < image.size(); i++)
    {
        double d = image.element(i).direction;
        if(d > max_direction)
            max_direction = d;
        if(d < min_direction)
            min_direction = d;
    }

    std::cout << "max: " << max_direction << "; min: " << min_direction << std::endl;

    OddSquareKernel W_kernel(kernel_size);
    Image<DirectionalFieldElement> result = with_filter<AvgDFFilter>(image, W_kernel);
    return result;
}

template<typename IP0, typename IP1>
void draw_block_directional_field(QGraphicsScene* scene, const Image<uint8_t, IP0>& original_image, const Image<DirectionalFieldElement, IP1>& block_directional_field, int32_t scale, int32_t block_x, int32_t block_y)
{
    scene->clear();
    //scene->addPixmap(QPixmap::fromImage(image_to_qImage(original_image)))->setScale(scale);

    double max_strength = 0;
    for(int32_t i = 0; i < block_directional_field.size(); i++)
        if(block_directional_field.element(i).coherence > max_strength)
            max_strength = block_directional_field.element(i).coherence;

    const int32_t len = static_cast<int32_t>(scale * std::min(block_x, block_y) * 0.7);
    QPen pen(QBrush(QColor(255, 0, 0)), 1.0);

    QPen blockCoherencePen(QBrush(QColor(0, 200, 0)), 1.0);

    for(int32_t x = 0; x < block_directional_field.width(); x++)
    {
        for(int32_t y = 0; y < block_directional_field.height(); y++)
        {
            double s = block_directional_field.element(x, y).coherence;
            int blockColor = static_cast<int>(s / max_strength * 255);
            scene->addRect(x * block_x * scale, y * block_y * scale, block_x * scale, block_y * scale)->setBrush(QBrush(QColor(blockColor, blockColor, blockColor)));
        }
    }

    for(int32_t x = 0; x < block_directional_field.width(); x++)
    {
        for(int32_t y = 0; y < block_directional_field.height(); y++)
        {
            double r = block_directional_field.element(x, y).direction;
            double s = block_directional_field.element(x, y).coherence;

            QLineF l(0, 0, len * cos(r) * (s / max_strength), len * sin(r) * (s / max_strength));
            auto line = scene->addLine(l, pen);
            line->setPos(x * block_x * scale + scale * block_x / 2, y * block_y * scale + scale * block_y / 2);
        }
    }
}

void MainWindow::on_actionOpen_image_triggered()
{
    /*QString filePath = QFileDialog::getOpenFileName(this, "Open image", "", "Images (*.bmp)");
    if(filePath.isNull())
        return;*/

    QString filePath = "cut.bmp";
    QString maskFilePath = "cut_3.bmp";
    //QString maskFilePath = filePath;

    ui->originalImage->scene()->clear();
    ui->gradientFieldImage->scene()->clear();
    ui->directionFieldImage->scene()->clear();

    //ui->gradientImage->scale(10, 10);

    QImage qImage(filePath);
    Image<uint8_t> image = qImage_to_image<uint8_t>(qImage, q2i_conv_func_color);

    /*Image<GradElement> grad_data = grad(image);
    Image<double, ImageView<double, ImageData<GradElement>>> grad_module = grad_data.template view_as<double>(gradViewArg);

    Image<uint8_t> grad_module_image = to_gray_image(grad_module);
    QImage grad_module_qImage = image_to_qImage(grad_module_image);

    Image<int32_t> laplacian_image = laplacian(image);
    QImage laplacian_qImage = image_to_qImage(to_gray_image(laplacian_image));

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    Image<DirectionalFieldElement> df_image = directional_field(image, 3);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

    std::cout << "duration: " << duration << std::endl;

    QImage df_qImage = image_to_qImage(to_gray_image(df_image.view_as<double>(directionalFieldViewDirection)));*/


    for(int32_t x = 0; x < image.width(); x++)
    {
        for(int32_t y = 0; y < image.height(); y++)
        {
            //image.element(x, y) = 128 + 125 * sin(x * 0.50 + y * 0.5) /** cos(y * 0.5)*/;
        }
    }


    QImage qMaskImage(maskFilePath);
    Image<uint8_t> maskImage = qImage_to_image<uint8_t>(qMaskImage, q2i_conv_func_color);
    auto mask_image_double = maskImage.convert_with<double>([](const auto& e){ return static_cast<double>(e); });
    auto mask_ft_image = fourier_transform(mask_image_double, true);
    auto mask_gauss = gaussian_hpf(image.width(), image.height(), 1, 100);
    //auto mask_ft_transformed = multiply(mask_ft_image, mask_gauss);
    auto& mask_ft_transformed = mask_ft_image;
    auto mask_ft_energy_image = mask_ft_transformed.template convert_with<double>([](const std::complex<double>& e){ return sqrt(pow(e.real(), 2) + pow(e.imag(), 2)); });
    auto mask_ft_scaled = log_transform(mask_ft_energy_image, 0.5);
    QImage mask_ft_qImage = image_to_qImage(to_gray_image(mask_ft_scaled));

    //auto gauss = gaussian_lpf(image.width(), image.height(), 1, 100);
    //auto gauss = gaussian_hpf(image.width(), image.height(), 1, 100);
    auto gauss = rot_gaussian_hpf(image.width(), image.height(), 1000);

    auto image_double = image.convert_with<double>([](const auto& e){ return static_cast<double>(e); });
    auto ft_image = fourier_transform(image_double, true);
    //auto ft_transformed = multiply(ft_image, gauss);
    auto ft_transformed = subtract(ft_image, mask_ft_image);
    //auto& ft_transformed = ft_image;

    auto ft_energy_image = ft_transformed.template convert_with<double>([](const std::complex<double>& e){ return sqrt(pow(e.real(), 2) + pow(e.imag(), 2)); });
    //ft_energy_image.element(image.width() / 2, image.height() / 2) = 0;
    //ft_energy_image.element(0, 0) = 0;
    auto ft_scaled = log_transform(ft_energy_image, 0.5);
    QImage ft_qImage = image_to_qImage(to_gray_image(ft_scaled));

    auto ift_image = inverse_fourier_transform(ft_transformed, true);
    //auto ift_real_image = ift_image.template convert_with<double>([](const std::complex<double>& e) { return e.real(); });
    //QImage ift_qImage = image_to_qImage(to_gray_image(ift_real_image));
    auto ift_real_image = ift_image.template convert_with<uint8_t>([](const std::complex<double>& e) { return static_cast<uint8_t>(e.real()); });
    QImage ift_qImage = image_to_qImage(ift_real_image);

    qImage = image_to_qImage(image);

    //QImage gauss_qImage = image_to_qImage(to_gray_image(gauss));
    //ui->gradientFieldImage->scene()->addPixmap(QPixmap::fromImage(gauss_qImage));

    ui->originalImage->scene()->addPixmap(QPixmap::fromImage(qImage))->setPos(0, 0);
    ui->originalImage->scene()->addPixmap(QPixmap::fromImage(ft_qImage))->setPos(image.width(), 0);
    ui->originalImage->scene()->addPixmap(QPixmap::fromImage(ift_qImage))->setPos(image.width() * 2, 0);

    ui->originalImage->scene()->addPixmap(QPixmap::fromImage(qMaskImage))->setPos(0, image.height());
    ui->originalImage->scene()->addPixmap(QPixmap::fromImage(mask_ft_qImage))->setPos(image.width(), image.height());

    //ui->originalImage->scene()->addPixmap(QPixmap::fromImage(grad_module_qImage))->setPos(image.width(), 0);
    //ui->originalImage->scene()->addPixmap(QPixmap::fromImage(df_qImage))->setPos(image.width() * 2, 0);
    ui->originalImage->setMinimumHeight(image.height());

   /* int block_size = 4;
    int image_scale = 2;
    auto block_df_data = block_directional_field(image, block_size, block_size);
    draw_block_directional_field(ui->directionFieldImage->scene(),
                               image,
                               block_df_data,
                               image_scale, block_size, block_size);

    auto avg_df_data = average_directional_field(block_df_data, 11);
    draw_block_directional_field(ui->gradientFieldImage->scene(),
                               image,
                               avg_df_data,
                               image_scale, block_size, block_size);*/
}













