#pragma once
#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <tuple>
#include <vector>

#include "config.hpp"

class disjoint_set
{
    public:
    // using size_type = size_t;
    disjoint_set(size_type size) noexcept : _parents(std::make_unique<size_type[]>(size)), _ranks(std::make_unique<size_type[]>(size)), _size(size)
    {
        std::iota(_parents.get(), _parents.get() + size, 0);
        std::iota(_ranks.get(), _ranks.get() + size, 0);
    }
    [[nodiscard]] size_type find(size_type x) noexcept { return x == _parents[x] ? x : (_parents[x] = find(_parents[x])); }
    [[nodiscard]] size_type find(size_type x) const noexcept { return x == _parents[x] ? x : find(_parents[x]); }
    [[nodiscard]] constexpr size_type size() const noexcept { return _size; }
    [[nodiscard]] bool is_eq(size_type x, size_type y) const noexcept { return find(x) == find(y); }
    [[nodiscard]] bool is_eq(size_type x, size_type y) noexcept { return find(x) == find(y); }
    bool merge(size_type x, size_type y) noexcept
    {
        x = find(x);
        y = find(y);
        if (_ranks[x] <= _ranks[y])
            _parents[x] = y;
        else
            _parents[y] = x;
        if (x == y)
            return false;
        else if (_ranks[x] == _ranks[y])
            _ranks[y]++;
        return true;
    }

    private:
    std::unique_ptr<size_type[]> _parents;
    std::unique_ptr<size_type[]> _ranks;
    size_type _size;
};

class eular_tour_forest
{
    public:
    eular_tour_forest(size_type size) noexcept : _size(size), _next(std::make_unique<size_type[]>(size))
    {
        std::iota(_next.get(), _next.get() + size, 0);
    }
    [[nodiscard]] constexpr size_type size() const noexcept { return _size; }
    [[nodiscard]] size_type next(size_type x) const noexcept { return _next[x]; }
    void merge(size_type x, size_type y) noexcept
    {
        std::swap(_next[x], _next[y]);
    }

    private:
    std::unique_ptr<size_type[]> _next;
    size_type _size;
};

class link_cut_tree
{
    public:

    private:
    std::unique_ptr<std::array<size_type, 2>[]> children;
    std::unique_ptr<size_type[]> father;
    std::unique_ptr<bool[]> tag;
    std::unique_ptr<size_type[]> weight;
    std::unique_ptr<size_type[]> id;

    public:
    std::map<std::tuple<size_type, size_type, size_type>, std::vector<std::pair<size_type, size_type>>> points;

    public:
    link_cut_tree(size_type num_time, size_type num_vert, std::vector<std::tuple<size_type, size_type, size_type, size_type>> &&temporal_edges) : children(std::make_unique<std::array<size_type, 2>[]>(num_vert + temporal_edges.size())), father(std::make_unique<size_type[]>(num_vert + temporal_edges.size())), tag(std::make_unique<bool[]>(num_vert + temporal_edges.size())), weight(std::make_unique<size_type[]>(num_vert + temporal_edges.size())), id(std::make_unique<size_type[]>(num_vert + temporal_edges.size()))
    {
        std::ranges::sort(temporal_edges,{},[](const auto &x){
            auto [u,v,t1,t2] = x;
            return std::tuple{t1,-t2,u,v};
        });
        auto intervals = std::vector<std::optional<size_type>>(temporal_edges.size());
        for (size_type i = 0; i < temporal_edges.size(); i++)
        {
            auto [u, v, t1, t2] = temporal_edges[i];
            size_type edge_id = num_vert + i;
            weight[edge_id] = num_time * num_time - t2 * num_time - t1;
            if (find(u) != find(v))
            {
                link(u, edge_id);
                link(v, edge_id);
                // intervals[i]=std::numeric_limits<size_type>::max();
                intervals[i] = num_time;
            }
            else
            {
                split(u, v);
                size_type cur = id[v];
                if (weight[edge_id] < weight[cur])
                {
                    cut(std::get<0>(temporal_edges[cur - num_vert]), cur);
                    cut(std::get<1>(temporal_edges[cur - num_vert]), cur);
                    link(u, edge_id);
                    link(v, edge_id);
                    // intervals[i]=std::numeric_limits<size_type>::max();
                    intervals[i] = num_time;
                    intervals[cur - num_vert] = t2 - 1;
                }
            }
        }
        for (size_type i = 0; i < temporal_edges.size(); i++)
        {
            auto interval = intervals[i];
            if (interval.has_value())
            {
                auto [u, v, t1, t2] = temporal_edges[i];
                points[{t1, t2, interval.value()}].push_back({u, v});
            }
        }
    }

    bool is_upper_child(size_type x) { return children[father[x]][1] == x; }
    bool is_root(size_type x) { return children[father[x]][0] != x && children[father[x]][1] != x; }
    void push_up(size_type x)
    {
        id[x] = x;
        if (children[x][0] && weight[id[children[x][0]]] > weight[id[x]])
            id[x] = id[children[x][0]];
        if (children[x][1] && weight[id[children[x][1]]] > weight[id[x]])
            id[x] = id[children[x][1]];
    }
    void rev(size_type x)
    {
        tag[x] = !tag[x];
        std::swap(children[x][0], children[x][1]);
    }
    void push_down(size_type x)
    {
        if (tag[x])
        {
            if (children[x][0])
                rev(children[x][0]);
            if (children[x][1])
                rev(children[x][1]);
            tag[x] = false;
        }
    }
    void update(size_type x)
    {
        if (!is_root(x))
            update(father[x]);
        push_down(x);
    }
    void rotate(size_type x)
    {
        size_type y = father[x];
        size_type z = father[y];
        size_type chx = is_upper_child(x) ? 1 : 0;
        size_type chy = is_upper_child(y) ? 1 : 0;
        father[x] = z;
        if (!is_root(y))
            children[z][chy] = x;
        children[y][chx] = children[x][chx ^ 1];
        father[children[x][chx ^ 1]] = y;
        children[x][chx ^ 1] = y;
        father[y] = x;
        push_up(y);
        push_up(x);
        if (z)
            push_up(z);
    }
    void splay(size_type x)
    {
        update(x);
        for (size_type f = father[x]; f = father[x], !is_root(x); rotate(x))
            if (!is_root(f))
                rotate(is_upper_child(x) == is_upper_child(f) ? f : x);
    }
    void access(size_type x)
    {
        for (size_type f = 0; x; f = x, x = father[x])
        {
            splay(x);
            children[x][1] = f;
            push_up(x);
        }
    }
    void make_root(size_type x)
    {
        access(x);
        splay(x);
        rev(x);
    }
    size_type find(size_type x)
    {
        access(x);
        splay(x);
        while (children[x][0])
        {
            x = children[x][0];
        }
        splay(x);
        return x;
    }
    void split(size_type x, size_type y)
    {
        make_root(x);
        access(y);
        splay(y);
    }
    void link(size_type x, size_type y)
    {
        make_root(x);
        father[x] = y;
    }
    void cut(size_type x, size_type y)
    {
        split(x, y);
        children[y][0] = father[x] = 0;
        push_up(y);
    }
};

//! Important: This function require temporal edges to be sorted
inline auto temporal_spanning_forest(size_type num_time, size_type num_vert, std::vector<std::tuple<size_type, size_type, size_type, size_type>> &&temporal_edges) -> std::map<std::tuple<size_type, size_type, size_type>, std::vector<std::pair<size_type, size_type>>>
{
    auto index = link_cut_tree{num_time, num_vert, std::move(temporal_edges)};
    return index.points;
}
