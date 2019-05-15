#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <algorithm>
#include <limits>
#include <stdint.h>
#include <functional>
#include <QAction>
#include <math.h>
#include <QSet>
#include <QDebug>
#include <optional>

struct point
{
    int32_t x, y;
    point(int32_t _x, int32_t _y)
        : x(_x), y(_y)
    {}

    point()
        : x(0), y(0)
    {}
};

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

class ImageOperations
{
public:
    Image binary_image(const Image& image, uint8_t threshold)
    {
        Image result(image.width(), image.height());
        for(int32_t i = 0; i < image.size(); i++)
        {
            result.element(i) = image.element(i) > threshold ? 255 : 0;
        }
        return result;
    }

    uint8_t q2i_conv_func_color(const QImage& qImage, int32_t x, int32_t y)
    {
        return static_cast<uint8_t>(qImage.pixelColor(x, y).red());
    }

    uint i2q_conv_func(const uint8_t& e)
    {
        return qRgb(e, e, e);
    }

    QImage image_to_qImage(const Image& image)
    {
        QImage::Format format = QImage::Format_RGB888;
        QImage qImage(image.width(), image.height(), format);
        for(int32_t x = 0; x < image.width(); x++)
        {
            for(int32_t y = 0; y < image.height(); y++)
            {
                qImage.setPixel(x, y, i2q_conv_func(image.element(x, y)));
            }
        }
        return qImage;
    }


    Image qImage_to_image(const QImage& qImage)
    {
        Image image(qImage.width(), qImage.height());
        for(int32_t x = 0; x < qImage.width(); x++)
            for(int32_t y = 0; y < qImage.height(); y++)
                image.element(x, y) = q2i_conv_func_color(qImage, x, y);
        return image;
    }

    template<typename EmptyTest>
    Image filtration(const Image& image, const EmptyTest& empty_test, const uint8_t& empty_value)
    {
        const std::vector<point> frame =
        {
            point(-3, -3), point(-2, -3), point(-1, -3), point(0, -3), point(1, -3), point(2, -3), point(3, -3),
            point(-3, -2),/* ___________________________________________________________________*/ point(3, -2),
            point(-3, -1),/* ___________________________________________________________________*/ point(3, -1),
            point(-3, 0),/* _____________________________________________________________________*/ point(3, 0),
            point(-3, 1),/* _____________________________________________________________________*/ point(3, 1),
            point(-3, 2),/* _____________________________________________________________________*/ point(3, 2),
            point(-3, 3), point(-2, 3),   point(-1, 3),  point(0, 3),  point(1, 3),  point(2, 3),   point(3, 3)
        };

        const std::vector<point> mask =
        {
            point(-3, -3), point(-2, -3), point(-1, -3), point(0, -3), point(1, -3), point(2, -3), point(3, -3),
            point(-3, -2), point(-2, -2), point(-1, -2), point(0, -2), point(1, -2), point(2, -2), point(3, -2),
            point(-3, -1), point(-2, -1), point(-1, -1), point(0, -1), point(1, -1), point(2, -1), point(3, -1),
            point(-3, 0), point(-2, 0), point(-1, 0), point(0, 0), point(1, 0), point(2, 0), point(3, 0),
            point(-3, 1), point(-2, 1), point(-1, 1), point(0, 1), point(1, 1), point(2, 1), point(3, 1),
            point(-3, 2), point(-2, 2), point(-1, 2), point(0, 2), point(1, 2), point(2, 2), point(3, 2),
            point(-3, 3), point(-2, 3), point(-1, 3), point(0, 3), point(1, 3), point(2, 3), point(3, 3)
        };

        Image clean_img = image.clone();
        for(int32_t x = 0; x < clean_img.width(); x++)
            for(int32_t y = 0; y < clean_img.height(); y++)
            {
                int count = 0;
                for(uint32_t index = 0; index < frame.size(); index++)
                {
                    count++;
                    uint8_t current = image.element_or(x + frame[index].x, y + frame[index].y, empty_value);
                    if (!empty_test(current))
                    {
                        count = 0;
                        break;
                    }
                }
                if (count != 0)
                {
                    for(int32_t index = 0; index < mask.size(); index++)
                    {
                        uint8_t current = image.element_or(x + mask[index].x, y + mask[index].y, empty_value);
                        if (!empty_test(current))
                        {
                            clean_img.element(x + mask[index].x, y + mask[index].y) = 255;
                        }
                    }
                }
            }

        return clean_img;
    }

    Image fragment(const Image& image, const uint8_t& value)
    {
        Image _fragmentImage(value, value);
        for(int32_t x = 0; x < value; x++)
            for(int32_t y = 0; y < value; y++)
            {
                _fragmentImage.element(x, y) = image.element(x, y);
            }
        return _fragmentImage;
    }

    QPoint find_blackPoint(const Image& _fragmentImage)
    {
        for(int32_t x = 0; x < _fragmentImage.width(); x++)
            for(int32_t y = 0; y < _fragmentImage.height(); y++)
            {
                if(_fragmentImage.element_or(x, y, 255) == 0)
                {
                    qDebug() <<  "found black point(" << x << "," << y << ")";
                    return QPoint(x, y);
                }
            }
        qDebug() <<  "selected empty point";
        return QPoint(0, 0);
    }

    void chained_code(const Image& _fragmentImage, QPoint initial_point)
    {
        if(_fragmentImage.element_or(initial_point.x(), initial_point.y(), 255) == 255)
        {
            qDebug() <<  "selected empty point";
            return;
        }

        const std::vector<QPoint> neighbours_pattern =
        {
            QPoint(1, 0), QPoint(1, 1),
            QPoint(0, 1), QPoint(-1, 1),
            QPoint(-1, 0), QPoint(-1, -1),
            QPoint(0, -1), QPoint(1, -1)
        };

        std::vector<QPoint> visited;
        visited.push_back(initial_point);
        QPoint current = initial_point;
        QPoint next = {};
        uint32_t neighbour_index = 0;
        QString code;

        for(int32_t counter = 0; counter < 1000; counter++)
        {
            for(uint32_t i = 0; i < neighbours_pattern.size(); i++)
            {
                const QPoint& n = neighbours_pattern[i];

                if(_fragmentImage.element_or(current.x() + n.x(), current.y() + n.y(), 255) == 0 && !(std::find(visited.begin(), visited.end(), current + n) != visited.end()))
                {
                    next = current + n;
                    neighbour_index = i;
                    break;
                }
            }

            if(_fragmentImage.element_or(next.x(), next.y(), 255) == 255)
            {
                qDebug() <<  "no points left";
                break;
            }

            current = next;
            next = {};
            visited.push_back(current);
            code += QString::number(neighbour_index);
        }

        qDebug() << code <<  '\n';
    }
};

#endif // IMAGE_H










