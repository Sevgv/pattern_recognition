#ifndef IMAGE_QT_H
#define IMAGE_QT_H

#include "image.hpp"
#include <QImage>
#include <functional>
#include <exception>

template<class P>
P q2i_conv_func(const QImage&, int32_t, int32_t)
{
    throw std::invalid_argument("not implemented");
}

uint8_t q2i_conv_func_color(const QImage& qImage, int32_t x, int32_t y);

template<class E>
Image<E> qImage_to_image(const QImage& qImage,
                         std::function<E (const QImage&, int32_t, int32_t)> conv_func = q2i_conv_func<E>)
{
    Image<E> image(qImage.width(), qImage.height());
    for(int32_t x = 0; x < qImage.width(); x++)
        for(int32_t y = 0; y < qImage.height(); y++)
            image.element(x, y) = conv_func(qImage, x, y);
    return std::move(image);
}


template<class E>
uint i2q_conv_func(const E&);
//{
//    throw std::invalid_argument("not implemented");
//}


template<class E, class P, QImage::Format format = QImage::Format_RGB888>
QImage image_to_qImage(const Image<E, P>& image,
                       std::function<uint (const E&)> conv_func = i2q_conv_func<E>)
{
    QImage qImage(image.width(), image.height(), format);
    for(int32_t x = 0; x < image.width(); x++)
    {
        for(int32_t y = 0; y < image.height(); y++)
        {
            qImage.setPixel(x, y, conv_func(image.element(x, y)));
        }
    }
    return qImage;
}

#endif // IMAGE_QT_H
