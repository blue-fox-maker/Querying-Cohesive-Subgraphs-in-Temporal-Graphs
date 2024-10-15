#pragma once
#include <algorithm>
#include <concepts>

namespace fn
{

template <size_t N>
constexpr auto get = [](auto &&arg) { return std::get<N>(std::forward<decltype(arg)>(arg)); };

constexpr auto compose(auto &&fn1, auto &&fn2)
{
    return [&](auto &&...args) { return fn2(fn1(std::forward<decltype(args)>(args)...)); };
}

// constexpr auto compose(auto &&func)
// {
//     return [f = std::forward<decltype(func)>(func)](auto &&...args) {
//         return std::forward<decltype(f)>(f)(std::forward<decltype(args)>(args)...);
//     };
// }

// constexpr auto compose(auto &&func1, auto &&func2, auto &&...funcs)
// {
//     return compose([f1 = std::forward<decltype(func1)>(func1), f2 = std::forward<decltype(func2)>(func2)](auto &&...args) {
//         return std::forward<decltype(f2)>(f2)(std::forward<decltype(f1)>(f1)(std::forward<decltype(args)>(args)...));
//     },std::forward<decltype(funcs)>(funcs)...);
// }
} // namespace fn
