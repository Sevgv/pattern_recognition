#ifndef LAPLACIAN_HPP
#define LAPLACIAN_HPP

#include "filter.hpp"

struct LaplacianKernel: public BaseKernel
{
    LaplacianKernel()
    {
        _kernel.reserve(5);
#define k(x, y, w) _kernel.emplace_back(x, y, w)
        k(0, 0, 4);
        k(-1, -1, -1);
        k(-1, 1, -1);
        k(1, -1, -1);
        k(1, 1, -1);
#undef k
    }
};

/*template<typename E, typename P>
struct LaplacianFilter
{
    typedef LaplacianKernel Kernel;
    typedef SimpleCollect<Kernel, E, P> Collect;
};*/

template<typename I>
Image<int32_t> laplacian(const I& image)
{
    return with_filter<SimpleFilter, LaplacianKernel>(image);
}

#endif // LAPLACIAN_HPP
