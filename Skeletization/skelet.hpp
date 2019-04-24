#ifndef SKELET_HPP
#define SKELET_HPP

#include "image.hpp"
#include <vector>
#include <iostream>

template<typename EmptyTest>
uint32_t __skelet_count_neighbours(const Image& image, int32_t x, int32_t y, const EmptyTest& empty_test, const uint8_t& empty_value)
{
    uint32_t count = 0;
    for(int32_t i = -1; i <= 1; i++)
    {
        for(int32_t j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0)
                continue;

            if(!empty_test(image.element_or(x + i, y + j, empty_value)))
                count++;
        }
    }
    return count;
}

template<typename EmptyTest>
uint32_t __skelet_count_0_1_transitions(const Image& image, std::vector<point> pattern, const EmptyTest& empty_test, const uint8_t& empty_value, int32_t x, int32_t y)
{
    uint32_t count = 0;
    uint8_t last = image.element_or(x + pattern[0].x, y + pattern[0].y, empty_value);

    for(uint32_t index = 1; index < pattern.size(); index++)
    {
        uint8_t current = image.element_or(x + pattern[index].x, y + pattern[index].y, empty_value);

        if(empty_test(last) && !empty_test(current))
            count++;
        last = current;
    }

    return count;
}

template<typename EmptyTest>
auto skelet(const Image& original_image, const EmptyTest& empty_test, const uint8_t& empty_value) -> Image
{
    Image image = original_image.clone();

    const std::vector<point> transition_counter_pattern =
    {
        point(0, -1), point(1, -1), point(1, 0),
        point(1, 1), point(0, 1), point(-1, 1),
        point(-1, 0), point(-1, -1), point(0, -1)
    };

    std::vector<point> for_removal(image.size());

    auto check_cond = [&image, &empty_test, &empty_value](int32_t x, int32_t y) -> uint8_t
    {
        return empty_test(image.element_or(x, y, empty_value)) ? 0 : 1;
    };

    while(true)
    {
        for(int32_t x = 0; x < image.width(); x++)
        {
            for(int32_t y = 0; y < image.height(); y++)
            {
                uint32_t neighbours_count = __skelet_count_neighbours(image, x, y, empty_test, empty_value);
                uint32_t transitions_count = __skelet_count_0_1_transitions(image, transition_counter_pattern, empty_test, empty_value, x, y);
                uint32_t cond0 = check_cond(x + 0, y - 1) * check_cond(x + 1, y + 0) * check_cond(x + 0, y + 1);
                uint32_t cond1 = check_cond(x + 1, y + 0) * check_cond(x + 0, y + 1) * check_cond(x - 1, y + 0);

                if((neighbours_count >= 2 && neighbours_count <= 6) && transitions_count == 1 && cond0 == 0 && cond1 == 0 && !empty_test(image.element(x, y)))
                    for_removal.push_back(point(x, y));
            }
        }

        if(!for_removal.size())
            break;

        for(auto it = for_removal.begin(); it != for_removal.end(); it++)
            image.element(it->x, it->y) = empty_value;
        for_removal.clear();

        for(int32_t x = 0; x < image.width(); x++)
        {
            for(int32_t y = 0; y < image.height(); y++)
            {
                uint32_t neighbours_count = __skelet_count_neighbours(image, x, y, empty_test, empty_value);
                uint32_t transitions_count = __skelet_count_0_1_transitions(image, transition_counter_pattern, empty_test, empty_value, x, y);
                uint32_t cond0 = check_cond(x + 0, y - 1) * check_cond(x + 1, y + 0) * check_cond(x - 1, y + 0);
                uint32_t cond1 = check_cond(x + 0, y - 1) * check_cond(x + 0, y + 1) * check_cond(x - 1, y + 0);

                if((neighbours_count >= 2 && neighbours_count <= 6) && transitions_count == 1 && cond0 == 0 && cond1 == 0 && !empty_test(image.element(x, y)))
                    for_removal.push_back(point(x, y));
            }
        }

        if(!for_removal.size())
            break;

        for(auto it = for_removal.begin(); it != for_removal.end(); it++)
            image.element(it->x, it->y) = empty_value;
        for_removal.clear();

    }

    return image;
}

#endif // SKELET_HPP











