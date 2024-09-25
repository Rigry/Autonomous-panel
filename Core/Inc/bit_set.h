#pragma once

#include <algorithm>

// этот класс создавался для заполнения метатаблицы (понятная работа с битами)
template <class T>
struct bit_set {
    T value;
    constexpr bit_set (T value) : value{value} {}
    constexpr bool operator[] (T bit)
    {
        return value & (1 << bit);
    }
    constexpr void set (T bit)
    {
        value |= (1 << bit);
    }
};

