#ifndef FILTER_HPP
#define FILTER_HPP

#include <stdint.h>
#include <cmath>
#include <vector>
#include "image.hpp"

struct KernelElement
{
    KernelElement() = default;

    KernelElement(int32_t x_, int32_t y_, int32_t w_)
        : x(x_), y(y_), w(w_)
    {}

    int32_t x, y;
    int32_t w;
};

struct BaseKernel
{
    inline const std::vector<KernelElement>& elements() const
    {
        return _kernel;
    }

    inline int32_t norm() const
    {
        int32_t n = 0;
        for(auto e = _kernel.begin(); e != _kernel.end(); e++)
            n += e->w;
        return n;
    }
protected:
    std::vector<KernelElement> _kernel;
};

struct OddSquareKernel: public BaseKernel
{
    OddSquareKernel(uint32_t size)
    {
        _kernel.reserve(size * size);
        int32_t len = size / 2;
        for(int32_t x = -len; x <= len; x++)
            for(int32_t y = -len; y <= len; y++)
                _kernel.emplace_back(x, y, 1);
    }
};

template<typename K, typename I>
struct SimpleFilterAvgCorners
{
    typedef int32_t new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        double sum = 0;
        double inside_w = 0;
        double outside_w = 0;
        for(auto e = kernel.elements().begin(); e != kernel.elements().end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
            {
                outside_w += e->w;
                continue;
            }

            sum += image.element(kx, ky) * e->w;
            inside_w += e->w;
        }

        if(std::abs(outside_w) >= 0.0001 && std::abs(inside_w) >= 0.0001)
            sum = sum + (sum / inside_w) * outside_w;


        return static_cast<new_element_t>(sum);
    }
};

template<typename K, typename I>
struct SimpleFilter
{
    typedef int32_t new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        double sum = 0;
        for(auto e = kernel.elements().begin(); e != kernel.elements().end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
                continue;

            sum += image.element(kx, ky) * e->w;
        }

        return static_cast<new_element_t>(sum);
    }
};

template<typename K, typename I>
struct SimpleAvgFilter
{
    typedef uint8_t new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        double sum = 0;
        for(auto e = kernel.elements().begin(); e != kernel.elements().end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
                continue;

            sum += image.pixel(kx, ky) * e->w;
        }

        return static_cast<new_element_t>(sum / static_cast<double>(kernel.norm()));
    }
};



template<typename N, typename I>
Image<N> convert_to(const I& image, const std::function<N (const typename I::element_t&)>& conv_func)
{
    Image<N> result = Image<N>::new_with_data(image.width(), image.height());
    for(int32_t i = 0; i < image.size(); i++)
        result.element(i) = conv_func(image.element(i));
    return result;
}

template<template<class, class> class F, typename K, typename I>
Image<typename F<K, I>::new_element_t> with_filter(const I& image, const K& kernel, F<K, I>& filter)
{
    typedef F<K, I> filter_t;
    typedef typename filter_t::new_element_t new_element_t;
    Image<new_element_t> result(image.width(), image.height());

    for(int32_t x = 0; x < image.width(); x++)
        for(int32_t y = 0; y < image.height(); y++)
        {
            new_element_t t = filter.collect(image, kernel, x, y);
            result.element(x, y) = t;
        }

    return result;
}

template<template<class, class> class F, typename K, typename I>
Image<typename F<K, I>::new_element_t> with_filter(const I& image, const K& kernel)
{
    typedef F<K, I> filter_t;
    filter_t filter;
    return with_filter<F, K, I>(image, kernel, filter);
}

template<template<class, class> class F, typename K, typename I>
Image<typename F<K, I>::new_element_t> with_filter(const I& image)
{
    typedef F<K, I> filter_t;
    filter_t filter;
    K kernel;
    return with_filter<F, K, I>(image, kernel, filter);
}

template<typename I>
Image<uint8_t> to_gray_image(const I& image)
{
    typedef typename I::element_t element_t;
    int32_t size = image.width() * image.height();
    std::vector<element_t> tmp(size);
    uint8_t max_value = std::numeric_limits<uint8_t>::max();
    Image<uint8_t> result(image.width(), image.height());

    element_t min = std::numeric_limits<element_t>::max();
    for(int32_t i = 0; i < size; i++)
        if(image.element(i) < min)
            min = image.element(i);

    for(int32_t i = 0; i < size; i++)
        tmp[i] = image.element(i) - min;

    element_t max = *std::max_element(tmp.begin(), tmp.end());
    for(int32_t i = 0; i < static_cast<int32_t>(tmp.size()); i++)
        result.element(i) = static_cast<uint8_t>(static_cast<double>(tmp[i]) / static_cast<double>(max) * max_value);

    return result;
}

#endif // FILTER_HPP
