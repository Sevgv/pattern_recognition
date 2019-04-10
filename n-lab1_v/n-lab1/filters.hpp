#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <stdint.h>
#include <vector>

template<class P>
class Image;

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
    const std::vector<KernelElement>& kernel() const
    {
        return _kernel;
    }

    int32_t norm() const
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

template<class P>
struct SimpleAvgCollect
{
    typedef uint8_t new_element_t;

    new_element_t collect(const Image<P>& image, const std::vector<KernelElement>& kernel_elements, int32_t kernel_norm, int32_t x, int32_t y)
    {
        double sum = 0;
        for(auto e = kernel_elements.begin(); e != kernel_elements.end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
                continue;

            sum += image.pixel(kx, ky) * e->w;
        }

        return static_cast<new_element_t>(sum / static_cast<double>(kernel_norm));
    }
};

template<class P>
struct SmoothenFilter
{
    typedef OddSquareKernel Kernel;
    typedef SimpleAvgCollect<P> Collect;

    const Kernel& kernel() const
    {
        return _kernel;
    }

    SmoothenFilter(const Kernel& kernel)
        : _kernel(kernel)
    {

    }

private:
    Kernel _kernel;
};



struct StandartGradKernel
{
    StandartGradKernel()
    {
        _kernel.reserve(3 * 3);
        int32_t len = 3 / 2;
        for(int32_t x = -len; x <= len; x++)
            for(int32_t y = -len; y <= len; y++)
                _kernel.emplace_back(x, y, y == 0 ? 2 * x : x);
    }

    const std::vector<KernelElement>& kernel() const
    {
        return _kernel;
    }

    int32_t norm() const
    {
        int32_t n = 0;
        for(auto e = _kernel.begin(); e != _kernel.end(); e++)
            n += e->w;
        return n;
    }

private:
    std::vector<KernelElement> _kernel;
};

template<class P>
struct GradCollect
{
    typedef int32_t new_element_t;

    new_element_t collect(const Image<P>& image, const std::vector<KernelElement>& kernel_elements, int32_t, int32_t x, int32_t y)
    {
        double sum = 0;
        for(auto e = kernel_elements.begin(); e != kernel_elements.end(); e++)
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

template<class P>
struct GradFilter
{
    typedef StandartGradKernel Kernel;
    typedef GradCollect<P> Collect;
};

#endif // FILTERS_HPP
