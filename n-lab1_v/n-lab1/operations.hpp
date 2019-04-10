#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

#include "image.hpp"
#include <cmath>

template<typename I>
I scale(const I& image, int32_t k)
{
    I result(image.width() * k, image.height() * k);

    for(int32_t x = 0; x < image.width(); x++)
        for(int32_t y = 0; y < image.height(); y++)
        {
            for(int32_t kx = 0; kx < k; kx++)
                for(int32_t ky = 0; ky < k; ky++)
                {
                    result.element(k * x + kx, k * y + ky) = image.element(x, y);
                }

        }

    return result;
}

template<typename ImgA, typename ImgB>
auto subtract(const ImgA& imgA, const ImgB& imgB) -> Image<decltype (imgA.element(0) - imgB.element(0))>
{
    if(imgA.width() != imgB.width() || imgA.height() != imgB.height())
        throw std::invalid_argument("size mismatch");

    using new_element_t = decltype (imgA.element(0) - imgB.element(0));
    Image<new_element_t> result(imgA.width(), imgA.height());

    for(int32_t x = 0; x < result.width(); x++)
    {
        for(int32_t y = 0; y < result.height(); y++)
            result.element(x, y) = imgA.element(x, y) - imgB.element(x, y);
    }
    return result;
}

template<typename I>
Image<typename I::element_t> log_transform(const I& image, double c)
{
    Image<typename I::element_t> result(image.width(), image.height());
    for(int32_t i = 0; i < image.size(); i++)
    {
        auto e = image.element(i);
        auto t =  c * log(1 + image.element(i));;
        result.element(i) = c * log(1 + image.element(i));
    }
    return result;
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

#endif // OPERATIONS_HPP
