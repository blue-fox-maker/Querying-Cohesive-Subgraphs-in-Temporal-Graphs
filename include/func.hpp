#pragma once
#include <functional>

namespace fn
{

template <size_t N>
constexpr auto get = [](auto &&arg) { return std::get<N>(std::forward<decltype(arg)>(arg)); };

struct no_op{
  constexpr void operator()(...) const noexcept {}
};

}
