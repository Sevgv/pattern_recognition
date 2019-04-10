#ifndef DIRECTIONAL_FIELD_H
#define DIRECTIONAL_FIELD_H

#include "image.hpp"
#include "filter.hpp"
#include "grad.hpp"
#include <iostream>

struct DirectionalFieldElement
{
    double Gx, Gy;
    double direction;
    double length;
    double coherence;
};

double& directionalFieldViewGx(DirectionalFieldElement& e)
{
    return e.Gx;
}

double& directionalFieldViewGy(DirectionalFieldElement& e)
{
    return e.Gy;
}

double& directionalFieldViewDirection(DirectionalFieldElement& e)
{
    return e.direction;
}

double& directionalFieldViewLength(DirectionalFieldElement& e)
{
    return e.length;
}

double& directionalFieldViewCoherence(DirectionalFieldElement& e)
{
    return e.coherence;
}

template<typename K, typename I>
struct SquareSumFilter
{
    typedef typename I::element_t new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        double sum = 0;
        for(auto e = kernel.elements().begin(); e != kernel.elements().end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
                continue;

            new_element_t v = image.element(kx, ky);
            sum += v * v;
        }

        return static_cast<new_element_t>(sum / kernel.norm());
    }
};

template<typename K, typename I>
struct SquareSumGxyFilter
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

            const GradElement& v = image.element(kx, ky);
            sum += v.Dx * v.Dy;
        }

        return static_cast<new_element_t>(sum / kernel.norm());
    }
};

double compute_angle(double x, double y)
{
    if(x >= 0)
        return atan(y / x);
    if(x < 0 && y >= 0)
        return atan(y / x) + M_PI;
    return atan(y / x) - M_PI;
}

double compute_direction(double f)
{
    //return f + M_PI_2;
    if(f <= 0)
        return f + M_PI_2;
    return f - M_PI_2;
}

template<typename K, typename I>
struct DirectionalFieldFilter
{
    typedef DirectionalFieldElement new_element_t;

    new_element_t collect(const I& image, const K& kernel, int32_t x, int32_t y)
    {
        double Gxx = 0, Gyy = 0, Gxy = 0;
        for(auto e = kernel.elements().begin(); e != kernel.elements().end(); e++)
        {
            int32_t kx = x + e->x;
            int32_t ky = y + e->y;

            if(image.is_index_out_of_bounds(kx, ky))
                continue;

            const GradElement& v = image.element(kx, ky);
            Gxx += v.Dx * v.Dx;
            Gyy += v.Dy * v.Dy;
            Gxy += v.Dx * v.Dy;
        }

        Gxx /= kernel.norm();
        Gyy /= kernel.norm();
        Gxy /= kernel.norm();

        DirectionalFieldElement e;
        e.Gx = Gxx;
        e.Gy = Gyy;
        e.direction = compute_direction(0.5 * compute_angle(Gxx - Gyy, 2 * Gxy));
        e.length = sqrt(Gxx + Gyy);
        e.coherence = sqrt(pow(Gxx - Gyy, 2) + 4 * pow(Gxy, 2)) / (Gxx + Gyy);
        return e;
    }
};

template<typename I>
Image<DirectionalFieldElement> directional_field(const I& image, uint32_t grad_kernel_size = 5)
{
    Image<GradElement> grad_data = grad(image);
    OddSquareKernel W_kernel(grad_kernel_size);

    auto Gx_view = grad_data.view_as<int32_t>(gradViewDx);
    auto Gy_view = grad_data.view_as<int32_t>(gradViewDy);

    for(int32_t i = 0; i < Gx_view.size(); i++)
        if(Gx_view.element(i) < 0)
        {
            Gx_view.element(i) *= -1;
            Gy_view.element(i) *= -1;
        }

    Image<DirectionalFieldElement> result = with_filter<DirectionalFieldFilter>(grad_data, W_kernel);
    return result;
}

template<typename I>
Image<DirectionalFieldElement> block_directional_field(const I& image, int32_t block_x, int32_t block_y, uint32_t grad_kernel_size = 5)
{
    Image<GradElement> grad_data = grad(image);
    OddSquareKernel W_kernel(grad_kernel_size);

    auto Gx_view = grad_data.view_as<int32_t>(gradViewDx);
    auto Gy_view = grad_data.view_as<int32_t>(gradViewDy);

    for(int32_t i = 0; i < Gx_view.size(); i++)
        if(Gx_view.element(i) < 0)
        {
            Gx_view.element(i) *= -1;
            Gy_view.element(i) *= -1;
        }

    Image<DirectionalFieldElement> result(image.width() / block_x, image.height() / block_y);

    const int32_t block_area = block_x * block_y;

    for(int32_t x = 0; x < result.width(); x++)
        for(int32_t y = 0; y < result.height(); y++)
        {
            double Gxx = 0, Gyy = 0, Gxy = 0;
            int32_t bx_limit = std::min(x * block_x + block_x, image.width());
            int32_t by_limit = std::min(y * block_y + block_y, image.height());
            for(int32_t bx = x * block_x; bx < bx_limit; bx++)
                for(int32_t by = y * block_y; by < by_limit; by++)
                {
                    double Dx = Gx_view.element(bx, by);
                    double Dy = Gy_view.element(bx, by);
                    Gxx += Dx * Dx;
                    Gyy += Dy * Dy;
                    Gxy += Dx * Dy;
                }

            Gxx /= block_area;
            Gyy /= block_area;
            Gxy /= block_area;

            DirectionalFieldElement& e = result.element(x, y);
            e.Gx = Gxx;
            e.Gy = Gyy;
            e.direction = compute_direction(0.5 * compute_angle(Gxx - Gyy, 2 * Gxy));
            e.length = sqrt(Gxx + Gyy);
            if(Gxx + Gyy != 0)
                e.coherence = sqrt(pow(Gxx - Gyy, 2) + 4 * pow(Gxy, 2)) / (Gxx + Gyy);
            else
                e.coherence = sqrt(pow(Gxx - Gyy, 2) + 4 * pow(Gxy, 2));
        }
    return result;
}

/*template<typename I>
Image<DirectionalFieldElement> directional_field(const I& image)
{
    Image<GradElement> grad_data = grad(image);
    OddSquareKernel W_kernel(5);

    auto Gx_view = grad_data.view_as<int32_t>(gradViewDx);
    auto Gy_view = grad_data.view_as<int32_t>(gradViewDy);

    for(int32_t i = 0; i < Gx_view.size(); i++)
        if(Gx_view.element(i) < 0)
        {
            Gx_view.element(i) *= -1;
            Gy_view.element(i) *= -1;
        }

    Image<int32_t> Gxx = with_filter<SquareSumFilter>(Gx_view, W_kernel);
    Image<int32_t> Gyy = with_filter<SquareSumFilter>(Gy_view, W_kernel);
    Image<int32_t> Gxy = with_filter<SquareSumGxyFilter>(grad_data, W_kernel);

    Image<DirectionalFieldElement> result(image.width(), image.height());

    for(int32_t i = 0; i < result.size(); i++)
    {
        double gxx = Gxx.element(i);
        double gyy = Gyy.element(i);
        double gxy = Gxy.element(i);
        DirectionalFieldElement& e = result.element(i);
        e.direction = compute_direction(0.5 * compute_angle(gxx - gyy, 2 * gxy));
        e.coherence = sqrt(pow(gxx - gyy, 2) + 4 * pow(gxy, 2)) / (gxx + gyy);
    }

    return result;
}*/

#endif // DIRECTIONAL_FIELD_H














