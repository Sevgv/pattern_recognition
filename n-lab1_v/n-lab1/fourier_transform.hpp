#ifndef FOURIER_TRANSFORM_HPP
#define FOURIER_TRANSFORM_HPP

#include <complex>
#include <cmath>
#include "image.hpp"

template <typename I, typename CV>
Image<std::complex<CV>> __fourier_transform(const I& image, bool forward)
{
    Image<std::complex<CV>> result(image.width(), image.height());
    Image<std::complex<CV>> tmp(image.width(), image.height());

    struct ft_precalc
    {
        CV _cos, _sin;
    };

    std::vector<std::vector<ft_precalc>> precalc(image.width());
    for(int32_t u = 0; u < image.width(); u++)
    {
        precalc[u].resize(image.width());
        for(int32_t x = 0; x < image.width(); x++)
        {
            precalc[u][x]._cos = cos((2.0 * M_PI * u * x) / image.width());
            precalc[u][x]._sin = sin((2.0 * M_PI * u * x) / image.width()) * (forward ? -1 : 1);
        }
    }

    for(int32_t row = 0; row < image.height(); row++)
    {
        for(int32_t u = 0; u < image.width(); u++)
        {
            std::complex<CV> sum = 0;
            for(int32_t x = 0; x < image.width(); x++)
            {
                //CV real = cos(2 * M_PI * (static_cast<CV>(u * x) / image.width()));
                //CV imag = -sin(2 * M_PI * (static_cast<CV>(u * x) / image.width()));
                CV real = precalc[u][x]._cos;
                CV imag = precalc[u][x]._sin;
                sum += image.element(x, row) * std::complex<CV>(real, imag);
            }

            if(!forward)
                sum /= static_cast<CV>(image.width());
            tmp.element(u, row) = sum;
        }
    }

    precalc.resize(image.height());
    for(int32_t u = 0; u < image.height(); u++)
    {
        precalc[u].resize(image.height());
        for(int32_t x = 0; x < image.height(); x++)
        {
            precalc[u][x]._cos = cos((2.0 * M_PI * u * x) / image.height());
            precalc[u][x]._sin = sin((2.0 * M_PI * u * x) / image.height()) * (forward ? -1 : 1);
        }
    }

    for(int32_t col = 0; col < image.width(); col++)
    {
        for(int32_t u = 0; u < image.height(); u++)
        {
            std::complex<CV> sum = 0;
            for(int32_t y = 0; y < image.height(); y++)
            {
                //CV real = cos(2 * M_PI * (static_cast<CV>(u * y) / image.height()));
                //CV imag = -sin(2 * M_PI * (static_cast<CV>(u * y) / image.height()));
                CV real = precalc[u][y]._cos;
                CV imag = precalc[u][y]._sin;
                sum += tmp.element(col, y) * std::complex<CV>(real, imag);
            }

            if(!forward)
                sum /= static_cast<CV>(image.height());
            result.element(col, u) = sum;
        }
    }

    return result;
}

template <typename I, typename CV = double>
Image<std::complex<CV>> fourier_transform(const I& image, bool center = true)
{
    if(!center)
        return __fourier_transform<I, CV>(image, true);
    Image<typename I::element_t> centered(image.width(), image.height());
    for(int32_t x = 0; x < image.width(); x++)
    {
        for(int32_t y = 0; y < image.height(); y++)
        {
            centered.element(x, y) = image.element(x, y) * pow(-1, x + y);
        }
    }
    return __fourier_transform<I, CV>(centered, true);
}

template <typename I, typename CV = double>
Image<std::complex<CV>> inverse_fourier_transform(const I& image, bool center = true)
{
    auto result = __fourier_transform<I, CV>(image, false);
    if(center)
    {
        for(int32_t x = 0; x < result.width(); x++)
        {
            for(int32_t y = 0; y < result.height(); y++)
            {
                result.element(x, y) = result.element(x, y) * pow(-1, x + y);
            }
        }
    }
    return result;
}

template<typename ImgA, typename ImgB>
auto multiply(const ImgA& imgA, const ImgB& imgB) -> Image<decltype (imgA.element(0) * imgB.element(0))>
{
    if(imgA.width() != imgB.width() || imgA.height() != imgB.height())
        throw std::invalid_argument("size mismatch");

    using new_element_t = decltype (imgA.element(0) * imgB.element(0));
    Image<new_element_t> result(imgA.width(), imgA.height());

    for(int32_t x = 0; x < result.width(); x++)
    {
        for(int32_t y = 0; y < result.height(); y++)
            result.element(x, y) = imgA.element(x, y) * imgB.element(x, y);
    }
    return result;
}

Image<double> gaussian_lpf(int32_t width, int32_t height, double A, double sigma2)
{
    Image<double> result(width, height);
    int32_t center_x = width / 2, center_y = height / 2;
    for(int32_t u = 0; u < result.width(); u++)
    {
        for(int32_t v = 0; v < result.height(); v++)
            result.element(u, v) = A * exp(-(pow(u - center_x, 2.0) + pow(v - center_y, 2.0)) / (2.0 * sigma2));
    }
    return result;
}

/*Image<double> gaussian_hpf(int32_t width, int32_t height, double A_1, double A_2,  double sigma2_1, double sigma2_2)
{
    Image<double> result(width, height);
    int32_t center_x = width / 2, center_y = height / 2;
    for(int32_t u = 0; u < result.width(); u++)
    {
        for(int32_t v = 0; v < result.height(); v++)
        {
            double coord = -(pow(u - center_x, 2.0) + pow(v - center_y, 2.0));
            result.element(u, v) = A_1 * exp(coord / (2.0 * sigma2_1)) - A_2 * exp(coord / (2.0 * sigma2_2));
        }
    }
    return result;
}*/

Image<double> gaussian_hpf(int32_t width, int32_t height, double A,  double sigma2)
{
    Image<double> result(width, height);
    int32_t center_x = width / 2, center_y = height / 2;
    for(int32_t u = 0; u < result.width(); u++)
    {
        for(int32_t v = 0; v < result.height(); v++)
        {
            double coord = -(pow(u - center_x, 2.0) + pow(v - center_y, 2.0));
            result.element(u, v) = A * (1.0 - exp(coord / (2.0 * sigma2)));
        }
    }
    return result;
}

#endif // FOURIER_TRANSFORM_HPP
