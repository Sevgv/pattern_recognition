#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <algorithm>
#include <limits>
#include <stdint.h>
#include <functional>

template<typename E, typename P>
class Image;

template<typename E>
class ImageData
{
public:
    typedef E element_t;

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

    inline const element_t& element(int32_t index) const
    {
        return _data[index];
    }

    inline element_t& element(int32_t index)
    {
        return _data[index];
    }

private:
    int32_t _width, _height;
    std::vector<element_t> _data;
};

template<typename ViewElement, typename BaseProvider>
class ImageView
{
public:
    typedef ViewElement element_t;
    typedef typename BaseProvider::element_t base_element_t;
    typedef std::function<element_t& (base_element_t&)> view_conv_t;

    ImageView(Image<base_element_t, BaseProvider>& image, const view_conv_t& view_conv)
        : _image(image), _view_conv(view_conv)
    {

    }

    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;

    ImageView(ImageView&& other)
        : _image(other._image),
          _view_conv(other._view_conv)
    {

    }

    inline int32_t width() const
    {
        return _image.width();
    }

    inline int32_t height() const
    {
        return _image.height();
    }

    inline const element_t& element(int32_t index) const
    {
        return _view_conv(_image.element(index));
    }

    inline element_t& element(int32_t index)
    {
        return _view_conv(_image.element(index));
    }

private:
    Image<base_element_t, BaseProvider>& _image;
    view_conv_t _view_conv;
};

template<typename E, typename P = ImageData<E>>
class Image
{
public:
    typedef E element_t;
    typedef P provider_t;



    Image(const Image& other) = delete;
    Image& operator=(const Image& other) = delete;

    Image(Image&& other)
        : _provider(std::move(other._provider))
    {

    }

    Image(provider_t&& provider)
        : _provider(std::move(provider))
    {

    }

    Image(int32_t width, int32_t height)
        : _provider(ImageData<element_t>(width, height))
    {

    }

    Image<element_t> clone() const
    {
        ImageData<element_t> provider(width(), height());
        for(int32_t i = 0; i < size(); i++)
            provider.element(i) = element(i);
        return provider;
    }

    template<typename V>
    Image<V, ImageView<V, provider_t>> view_as(const typename ImageView<V, provider_t>::view_conv_t& view_conv)
    {
        ImageView<V, provider_t> view(*this, view_conv);
        return Image<V, ImageView<V, provider_t>>(std::move(view));
    }

    template<typename NE>
    Image<NE> convert_with(const std::function<NE (const element_t&)> conv_func) const
    {
        Image<NE, ImageData<NE>> result(width(), height());
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

    inline const element_t& element(int32_t index) const
    {
        return _provider.element(index);
    }

    inline element_t& element(int32_t index)
    {
        return _provider.element(index);
    }

    const element_t& element(int32_t x, int32_t y) const
    {
        return element(y * width() + x);
    }

    element_t& element(int32_t x, int32_t y)
    {
        return element(y * width() + x);
    }

    element_t element_or(int32_t x, int32_t y, const element_t& fallback) const
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
    provider_t _provider;
};

#endif // IMAGE_H










