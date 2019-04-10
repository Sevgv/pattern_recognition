#include "image_qt.hpp"

template<>
uint8_t q2i_conv_func(const QImage& qImage, int32_t x, int32_t y)
{
    return static_cast<uint8_t>(qImage.pixelIndex(x, y));
}


template<>
uint i2q_conv_func(const uint8_t& e)
{
    return qRgb(e, e, e);
}

uint8_t q2i_conv_func_color(const QImage& qImage, int32_t x, int32_t y)
{
    return static_cast<uint8_t>(qImage.pixelColor(x, y).red());
}
