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

inline uint qHash (const QPoint & key)
{
return qHash (QPair<int,int>(key.x(), key.y()) );
}

inline bool operator<(const QPoint& a, const QPoint& b)
{
return qHash(a) < qHash(b);
}

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

    void find_all_neighbours(const Image& skelet, const uint8_t empty_value, const uint8_t non_empty_value, const Image& visited_mask, QSet<QPoint>& neighbours, const QPoint point)
    {
        static const std::vector<QPoint> neighbours_pattern =
        {
            QPoint(1, 0), QPoint(1, 1),
            QPoint(0, 1), QPoint(-1, 1),
            QPoint(-1, 0), QPoint(-1, -1),
            QPoint(0, -1), QPoint(1, -1)
        };

        neighbours.insert(point);

        for(const QPoint& p : neighbours_pattern)
        {
            QPoint current = point + p;
            if(skelet.element_or(current.x(), current.y(), empty_value) == non_empty_value && visited_mask.element_or(current.x(), current.y(), true) == false && !neighbours.contains(current))
            {
                find_all_neighbours(skelet, empty_value, non_empty_value, visited_mask, neighbours, current);
            }
        }
    }

    Image filter_skelet(const Image& skelet, int32_t fragment_threshold, uint8_t empty_value = 255)
    {
        Image visited_mask(skelet.width(), skelet.height());
        for(int32_t i = 0; i < visited_mask.size(); i++)
            visited_mask.element(i) = false;

        Image result = skelet.clone();
        QSet<QPoint> neighbours;

        for(int32_t x = 0; x < skelet.width(); x++)
        {
            for(int32_t y = 0; y < skelet.height(); y++)
            {
                if(skelet.element(x, y) != 0 || visited_mask.element(x, y))
                    continue;

                neighbours.clear();
                find_all_neighbours(skelet, 255, 0, visited_mask, neighbours, QPoint(x, y));
                //qDebug() << neighbours.size();

                for(const QPoint& p : neighbours)
                    visited_mask.element(p.x(), p.y()) = true;

                if(neighbours.size() <= fragment_threshold)
                    for(const QPoint& p : neighbours)
                        result.element(p.x(), p.y()) = empty_value;
            }
        }

        return result;
    }

    void find_local_neighbours(const Image& image, const QPoint& point, const uint8_t empty_value, const uint8_t non_empty_value, QSet<QPoint>& neighbours)
    {
        static const std::vector<QPoint> neighbours_pattern =
        {
            QPoint(1, 0), QPoint(1, 1),
            QPoint(0, 1), QPoint(-1, 1),
            QPoint(-1, 0), QPoint(-1, -1),
            QPoint(0, -1), QPoint(1, -1)
        };

        for(const QPoint& p : neighbours_pattern)
        {
            QPoint current = point + p;
            if(image.element_or(current.x(), current.y(), empty_value) == non_empty_value)
                neighbours.insert(current);
        }
    }

    Image filter_useless_points_new(const Image& skelet, uint8_t empty_value = 255)
    {
        Image result = skelet.clone();

        static const std::vector<QPoint> neighbours_pattern =
        {
            QPoint(1, 0), QPoint(1, 1),
            QPoint(0, 1), QPoint(-1, 1),
            QPoint(-1, 0), QPoint(-1, -1),
            QPoint(0, -1), QPoint(1, -1)
        };

        for(int32_t x = 0; x < result.width(); x++)
            for(int32_t y = 0; y < result.height(); y++)
            {
                if(result.element(x, y) != 0)
                    continue;

                QPoint current = QPoint(x, y);
                QSet<QPoint> neighbours;
                find_local_neighbours(result, current, 255, 0, neighbours);

                QMap<QPoint, bool> marked;
                for(const QPoint& neighbour : neighbours)
                {
                    bool mark = false;
                    for(const QPoint& p : neighbours_pattern)
                        if(neighbours.contains(neighbour + p))
                        {
                            mark = true;
                            break;
                        }
                    marked[neighbour] = mark;
                }

                if(!marked.values().toSet().contains(false))
                {
                    result.element(x, y) = empty_value;
                }
            }
        return result;
    }

    Image filter_short_branches(const Image& skelet, int32_t branch_threshold, uint8_t empty_value = 255)
    {
        Image result = skelet.clone();

        for(int32_t x = 0; x < result.width(); x++)
            for(int32_t y = 0; y < result.height(); y++)
            {
                if(result.element(x, y) != 0)
                    continue;

                QPoint current = QPoint(x, y);
                QSet<QPoint> neighbours;
                find_local_neighbours(result, current, 255, 0, neighbours);
                if(neighbours.size() > 1)
                    continue;

                QPoint current_branch_point = *neighbours.begin();
                QSet<QPoint> branch_points;
                branch_points.insert(current);
                while(true)
                {
                    QSet<QPoint> current_branch_point_neighbours;
                    find_local_neighbours(result, current_branch_point, 255, 0, current_branch_point_neighbours);
                    current_branch_point_neighbours.subtract(branch_points);

                    if(current_branch_point_neighbours.size() == 1)
                    {
                        branch_points.insert(current_branch_point);
                        current_branch_point = *current_branch_point_neighbours.begin();
                    }
                    else
                        break;
                }

                if(branch_points.size() > branch_threshold)
                    continue;

                //qDebug() << branch_points.size();
                for(const QPoint& p : branch_points)
                    result.element(p.x(), p.y()) = empty_value;
            }
        return result;
    }

    std::vector<double> compute_central_moments(const Image& image, const QPoint& point, const int window_size)
    {
        auto m = [&](int p, int q)
        {
            double s = 0;
            for(int x = -window_size / 2.0; x < window_size / 2.0; x++)
                for(int y = -window_size / 2.0; y < window_size / 2.0; y++)
                    s += pow(x, p) * pow(y, q) * (image.element_or(point.x() + x, point.y() + y, 255) == 255 ? 0 : 1);
            return s;
        };

        double m10 = m(1, 0), m01 = m(0, 1), m00 = m(0, 0);
        double avg_x = m10 / (m00 != 0 ? m00 : 1);
        double avg_y = m01 / (m00 != 0 ? m00 : 1);

        auto u = [&](int p, int q)
        {
            double s = 0;
            for(int x = -window_size / 2.0; x < window_size / 2.0; x++)
                for(int y = -window_size / 2.0; y < window_size / 2.0; y++)
                    s += pow(x - avg_x, p) * pow(y - avg_y, q) * (image.element_or(point.x() + x, point.y() + y, 255) == 255 ? 0 : 1);
            return s;
        };

        std::vector<double> res(3);

        res[0] = u(0, 0);
        res[1] = u(1, 0);
        res[2] = u(0, 1);

        return res;
    }

    QImage match_process(const Image& _fragmentImage, QPoint initial_point, const int window_size)
    {
        std::vector<double> initial_moments = compute_central_moments(_fragmentImage, initial_point, window_size);

        Image result(_fragmentImage.width(), _fragmentImage.height());

        QImage QImage(_fragmentImage.width(), _fragmentImage.height(), QImage::Format_RGB888);
        for(int x = 0; x < _fragmentImage.width(); x++)
            for(int y = 0; y < _fragmentImage.height(); y++)
            {
                std::vector<double> data = compute_central_moments(_fragmentImage, QPoint(x, y), window_size);

                double sum = 0;
                for(int i = 0; i < data.size(); i++)
                    sum += pow(data[i] - initial_moments[i], 2);
                double error = sqrt(sum / data.size());

                result.element(x, y) = error;
            }

        double max_value = 0;
        for(int x = 0; x < _fragmentImage.width(); x++)
            for(int y = 0; y < _fragmentImage.height(); y++)
            {
                if(result.element(x, y) > max_value)
                    max_value = result.element(x, y);
            }

        for(int x = 0; x < _fragmentImage.width(); x++)
            for(int y = 0; y < _fragmentImage.height(); y++)
                QImage.setPixel(x, y, qRgb(0, 255 * (1 - result.element(x, y) / max_value), 0));

//        ui->fragmentImage->scene()->clear();
//        _qFragmentImage = ui->fragmentImage->scene()->addPixmap(QPixmap::fromImage(QImage));
//        _qFragmentImage->setScale(_fragmentScale);
        return QImage;
    }
};

#endif // IMAGE_H










