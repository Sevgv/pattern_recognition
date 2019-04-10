#ifndef SMOOTHEN_HPP
#define SMOOTHEN_HPP

#include <filter.hpp>

/*template<typename E, typename P>
struct SmoothenFilter
{
    typedef OddSquareKernel Kernel;
    typedef SimpleAvgFilter<Kernel, E, P> Collect;

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
};*/

#endif // SMOOTHEN_HPP
