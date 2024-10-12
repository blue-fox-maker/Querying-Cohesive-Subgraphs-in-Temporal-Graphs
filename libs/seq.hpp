// sequence algorithm on span
#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <type_traits>

// preorder tree view on span
template <typename T>
class tree_view
{
    std::span<T> data;

    public:
    using reference = T &;
    tree_view(std::span<T> data) : data(data) {}
    size_t size() { return data.size(); }

    bool empty() { return size() == 0; }
    bool is_leaf() { return size() == 1; }
    reference root() { return data[size() / 2]; }
    tree_view lower_child() { return data.first(size() / 2); }
    tree_view upper_child() { return data.last(size() >= 2 ? size() - size() / 2 - 1 : 0); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    auto rbegin() const { return data.rbegin(); }
    auto rend() const { return data.rend(); }

    operator std::span<T>() { return data; }
};

template <std::ranges::range R1, std::ranges::range R2>
auto kth_element(R1 &&rng1, R2 &&rng2, size_t k)
{
    assert(k < std::ranges::size(rng1) + std::ranges::size(rng2));
    if (std::ranges::size(rng1) == 0)
        return *(std::ranges::begin(rng2) + k);
    if (std::ranges::size(rng1) > std::ranges::size(rng2))
        return kth_element(std::forward<R2>(rng2), std::forward<R1>(rng1), k);
    if (k == 1)
        return std::max(*std::ranges::begin(rng1), *std::ranges::begin(rng2));
    size_t index1 = std::min(std::ranges::size(rng1) - 1, k / 2);
    size_t index2 = std::min(std::ranges::size(rng2) - 1, k / 2);
    if (*(std::ranges::begin(rng1) + index1) > *(std::ranges::begin(rng2) + index2))
        return kth_element(rng1, std::ranges::subrange(std::ranges::begin(rng2) + index2, std::ranges::end(rng2)), k - index2);
    else
        return kth_element(std::ranges::subrange(std::ranges::begin(rng1) + index1, std::ranges::end(rng1)), rng2, k - index1);
}
