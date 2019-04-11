#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <algorithm>
#include <limits>
#include <stdint.h>
#include <functional>

class Image;

class ImageData
{
public:  

    ImageData(int32_t width, int32_t height)
        : _width(width), _height(height)
    {
        _data.resize(width * height);
    }

    ImageData(const ImageData&) = delete;
    ImageData& operator=(const ImageData&) = delete;

    ImageData(ImageData&& other)
        : _width(other.width()),
          _height(other.height()),
          _data(std::move(other._data))
    {

    }

    inline int32_t width() const
    {
        return _width;
    }

    inline int32_t height() const
    {
        return _height;
    }

    inline const uint8_t& element(int32_t index) const
    {
        return _data[index];
    }

    inline uint8_t& element(int32_t index)
    {
        return _data[index];
    }

private:
    int32_t _width, _height;
    std::vector<uint8_t> _data;
};

class Image
{
public:

    Image(const Image& other) = delete;
    Image& operator=(const Image& other) = delete;

    Image(Image&& other)
        : _provider(std::move(other._provider))
    {

    }

    Image(ImageData&& provider)
        : _provider(std::move(provider))
    {

    }

    Image(int32_t width, int32_t height)
        : _provider(ImageData(width, height))
    {

    }

    Image clone() const
    {
        ImageData provider(width(), height());
        for(int32_t i = 0; i < size(); i++)
            provider.element(i) = element(i);
        return provider;
    }

    template<typename NE>
    Image convert_with(const std::function<NE (const uint8_t&)> conv_func) const
    {
        Image result(width(), height());
        for(int32_t i = 0; i < result.size(); i++)
            result.element(i) = conv_func(element(i));
        return result;
    }

    inline int32_t width() const
    {
        return _provider.width();
    }

    inline int32_t height() const
    {
        return _provider.height();
    }

    int32_t size() const
    {
        return width() * height();
    }

    inline const uint8_t& element(int32_t index) const
    {
        return _provider.element(index);
    }

    inline uint8_t& element(int32_t index)
    {
        return _provider.element(index);
    }

    const uint8_t& element(int32_t x, int32_t y) const
    {
        return element(y * width() + x);
    }

    uint8_t& element(int32_t x, int32_t y)
    {
        return element(y * width() + x);
    }

    uint8_t element_or(int32_t x, int32_t y, const uint8_t& fallback) const
    {
        if(!is_index_out_of_bounds(x, y))
            return element(x, y);
        return fallback;
    }

    bool is_index_out_of_bounds(int32_t x, int32_t y) const
    {
        return x < 0 || y < 0 || x >= width() || y >= height();
    }

private:
    ImageData _provider;
};

#endif // IMAGE_H










