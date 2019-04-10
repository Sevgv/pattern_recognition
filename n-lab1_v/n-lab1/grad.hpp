#ifndef GRAD_HPP
#define GRAD_HPP

#include "image.hpp"
#include "filter.hpp"
#include <cmath>

struct GradElement
{
    int32_t Dx, Dy;
    double m;
    double f;
};

struct GradXKernel: public BaseKernel
{
    GradXKernel()
    {
        /*_kernel.reserve(3 * 3);
        int32_t len = 3 / 2;
        for(int32_t x = -len; x <= len; x++)
            for(int32_t y = -len; y <= len; y++)
                _kernel.emplace_back(x, y, y == 0 ? 2 * x : x);*/

        _kernel.reserve(6);
#define k(x, y, w) _kernel.emplace_back(x, y, w)
        k(-1, -1, -1);
        k(-1,  0, -2);
        k(-1,  1, -1);
        k(1, -1, 1);
        k(1,  0, 2);
        k(1,  1, 1);
#undef k
    }
};

struct GradYKernel: public BaseKernel
{
    GradYKernel()
    {
        /*_kernel.reserve(3 * 3);
        int32_t len = 3 / 2;
        for(int32_t x = -len; x <= len; x++)
            for(int32_t y = -len; y <= len; y++)
                _kernel.emplace_back(x, y, x == 0 ? 2 * y : y);*/

        _kernel.reserve(6);
#define k(x, y, w) _kernel.emplace_back(x, y, w)
        k(-1, -1, -1);
        k(0,  -1, -2);
        k(1,  -1, -1);
        k(-1, 1, 1);
        k(0,  1, 2);
        k(1,  1, 1);
#undef k
    }
};


/*template<typename E, typename P>
Image<GradElement> grad(const Image<E, P>& image)
{
    Image<int32_t> gradx = with_filter<SimpleFilter, GradYKernel>(image);
    Image<int32_t> grady = with_filter<SimpleFilter, GradXKernel>(image);
    Image<GradElement> grad(image.width(), image.height());

    for(int32_t x = 0; x < grad.width(); x++)
    {
        for(int32_t y = 0; y < grad.height(); y++)
        {
            GradElement& e = grad.element(x, y);
            e.Dx = gradx.element(x, y);
            e.Dy = grady.element(x, y);
            e.m = sqrt(pow(e.Dx, 2) + pow(e.Dy, 2));
            //e.f = atan2(e.Dx, e.Dy);
            e.f = atan2(e.Dy, e.Dx);
        }
    }

    return grad;
}*/

template<typename I>
Image<GradElement> grad(const I& image)
{
    Image<int32_t> gradx = with_filter<SimpleFilterAvgCorners, GradXKernel>(image);
    Image<int32_t> grady = with_filter<SimpleFilterAvgCorners, GradYKernel>(image);
    Image<GradElement> grad(image.width(), image.height());

    for(int32_t x = 0; x < grad.width(); x++)
    {
        for(int32_t y = 0; y < grad.height(); y++)
        {
            GradElement& e = grad.element(x, y);
            e.Dx = gradx.element(x, y);
            e.Dy = grady.element(x, y);
            e.m = sqrt(pow(e.Dx, 2) + pow(e.Dy, 2));
            //e.f = atan2(e.Dx, e.Dy);
            e.f = atan2(e.Dy, e.Dx);
        }
    }

    return grad;
}

int32_t& gradViewDx(GradElement& e)
{
    return e.Dx;
}

int32_t& gradViewDy(GradElement& e)
{
    return e.Dy;
}

double& gradViewModule(GradElement& e)
{
    return e.m;
}

double& gradViewArg(GradElement& e)
{
    return e.f;
}

#endif // GRAD_HPP


















