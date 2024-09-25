#pragma once

#include <functional>

template<class...Args>
using Callback = std::function<void(Args...)>;

template<class Function, class...Args>
void execute (Function f, Args...args) {
    if (f)
        f(args...);
}

template<class Function, class...Args>
void execute_if (bool condition, Function f, Args...args) {
    if (condition)
        execute (f, args...);
}
