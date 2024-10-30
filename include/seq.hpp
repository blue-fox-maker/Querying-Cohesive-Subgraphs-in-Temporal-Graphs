#pragma once
#include <span>

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
