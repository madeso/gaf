#pragma once

#include <array>

 
template <typename T, typename ...V>
constexpr auto make_array(V... v)
{
    return std::array<T, sizeof...(v)>
    {
        v...,
    };
}
