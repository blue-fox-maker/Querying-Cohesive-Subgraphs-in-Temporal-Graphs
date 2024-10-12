#include "seq.hpp"
#include "config.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

template <typename TPoint, size_t Dimension = std::tuple_size_v<TPoint>>
struct orthogonal_search_tree_node
{
    // using size_type = size_t;
    using key_type = std::tuple_element_t<Dimension - 1, TPoint>;
    using value_type = std::pair<TPoint, size_type>;
    using point_type = TPoint;
    std::unique_ptr<orthogonal_search_tree_node> lower_child;
    std::unique_ptr<orthogonal_search_tree_node> upper_child;
    orthogonal_search_tree_node<TPoint, Dimension - 1> next_dim;
    key_type key;
    [[nodiscard]] static constexpr key_type at_dim(const value_type &p) noexcept
    {
        return std::get<Dimension - 1>(std::get<0>(p));
    }
    [[nodiscard]] constexpr bool is_leaf() const noexcept
    {
        return !lower_child && !upper_child;
    }
    [[nodiscard]] size_type num_node() const noexcept
    {
        return 1 + (lower_child ? lower_child->num_node() : 0) +
               next_dim.num_node() + (upper_child ? upper_child->num_node() : 0);
    }
    orthogonal_search_tree_node() = default;
    orthogonal_search_tree_node(std::vector<value_type> &&points)
    {
        auto unique_indice = std::vector<size_type>{0};
        std::ranges::sort(points, {}, at_dim);
        auto prev = at_dim(points[0]);
        for (size_type i = 0; i < points.size(); prev = at_dim(points[i]), i++)
        {
            if (at_dim(points[i]) != prev)
                unique_indice.push_back(i);
        }
        build_tree(points, unique_indice, 0);
    }
    orthogonal_search_tree_node(std::span<value_type> points,
                                std::span<size_type> unique_indice, size_type base_index)
    {
        build_tree(points, unique_indice, base_index);
    }
    void build_tree(std::span<value_type> points,
                    std::span<size_type> unique_indice, size_type base_index)
    {
        assert(!unique_indice.empty());
        auto mid = (unique_indice.size() - 1) / 2;
        key = at_dim(points[unique_indice[mid] - base_index]);
        if (unique_indice.size() > 1)
        {
            auto mid_index = unique_indice[mid + 1] - base_index;
            if (auto lower_indice = unique_indice.subspan(0, mid + 1);
                !lower_indice.empty())
                lower_child = std::make_unique<orthogonal_search_tree_node<TPoint, Dimension>>(points.subspan(0, mid_index), lower_indice, base_index);
            if (auto upper_indice = unique_indice.subspan(mid + 1);
                !upper_indice.empty())
                upper_child = std::make_unique<orthogonal_search_tree_node<TPoint, Dimension>>(points.subspan(mid_index), upper_indice, mid_index + base_index);
        }
        auto temp = std::vector<value_type>{};
        std::ranges::copy(points, std::back_inserter(temp));
        next_dim = orthogonal_search_tree_node<TPoint, Dimension - 1>{std::move(temp)};
    }
    template <typename Fn>
    void query(const point_type &lower, const point_type &upper, Fn &&func)
    {
        auto lower_key = std::get<Dimension - 1>(lower);
        auto upper_key = std::get<Dimension - 1>(upper);
        if (is_leaf())
        {
            if (lower_key <= key && key <= upper_key)
                return next_dim.query(lower, upper, func);
        }
        else if (upper_key < key)
        {
            if (lower_child)
                lower_child->query(lower, upper, func);
        }
        else if (lower_key > key)
        {
            if (upper_child)
                upper_child->query(lower, upper, func);
        }
        else
        {
            auto lower_cur = lower_child.get();
            while (lower_cur)
            {
                if (lower_cur->is_leaf())
                {
                    if (lower_key <= lower_cur->key)
                        lower_cur->next_dim.query(lower, upper, func);
                    break;
                }
                else if (lower_key <= lower_cur->key)
                {
                    if (lower_cur->upper_child)
                        lower_cur->upper_child->next_dim.query(lower, upper, func);
                    lower_cur = lower_cur->lower_child.get();
                }
                else
                    lower_cur = lower_cur->upper_child.get();
            }
            auto upper_cur = upper_child.get();
            while (upper_cur)
            {
                if (upper_cur->is_leaf())
                {
                    if (upper_key >= upper_cur->key)
                        upper_cur->next_dim.query(lower, upper, func);
                    break;
                }
                else if (upper_key > upper_cur->key)
                {
                    if (upper_cur->lower_child)
                        upper_cur->lower_child->next_dim.query(lower, upper, func);
                    upper_cur = upper_cur->upper_child.get();
                }
                else
                    upper_cur = upper_cur->lower_child.get();
            }
        }
    }
};

template <typename TPoint>
struct orthogonal_search_tree_node<TPoint, 1>
{
    using point_type = TPoint;
    // using size_type = size_t;
    using key_type = std::tuple_element_t<0, TPoint>;
    using value_type = std::pair<TPoint, size_type>;
    [[nodiscard]] size_type num_node() const noexcept
    {
        return _data.size();
    }
    orthogonal_search_tree_node() = default;
    orthogonal_search_tree_node(std::vector<value_type> &&points)
    {
        std::ranges::sort(points, {}, [](const auto &x) { return std::get<0>(x); });
        std::ranges::move(points | std::views::transform([](const auto &x) { return std::make_pair(std::get<0>(x.first), x.second); }), std::back_inserter(_data));
    }
    template <typename Fn>
    void query(const point_type &lower, const point_type &upper, Fn &&func)
    {
        auto lower_iter = std::ranges::lower_bound(_data, std::get<0>(lower), {}, [](const auto &x) { return std::get<0>(x); });
        auto upper_iter = std::ranges::upper_bound(_data, std::get<0>(upper), {}, [](const auto &x) { return std::get<0>(x); });
        std::ranges::for_each(lower_iter, upper_iter, func, [](const auto &x) { return std::get<1>(x); });
    }
    std::vector<std::pair<key_type, size_type>> _data;
};

template <typename TPoint, typename TValue>
class orthogonal_search_tree
{
    public:
    using point_type = TPoint;
    using value_type = TValue;
    // using size_type = size_t;

    orthogonal_search_tree() = default;
    template <std::ranges::input_range R>
    orthogonal_search_tree(R &&rng)
    {
        auto points = std::vector<std::pair<point_type, size_type>>{};
        size_type count = 0;
        for (auto &&[k, v] : rng)
        {
            points.push_back({k, count++});
            _data.push_back(v);
        }
        assert(!points.empty());
        _root = std::make_unique<orthogonal_search_tree_node<TPoint>>(std::move(points));
    }
    template <typename Fn>
    void query(const point_type &lower, const point_type &upper, Fn &&func)
    {
        if (_root)
            _root->query(lower, upper, [&](const size_type idx) { func(_data[idx]); });
    }
    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return _data.size();
    }
    [[nodiscard]] constexpr size_type num_node() const noexcept
    {
        return _root->num_node();
    }
    [[nodiscard]] auto &points() const noexcept
    {
        return _data;
    }

    private:
    std::unique_ptr<orthogonal_search_tree_node<TPoint>> _root;
    std::vector<value_type> _data;
};

// FOR DEBUG
// template <typename TPoint, typename TValue = void>
// class range_debug_tree
// {
//     public:
//     using point_type = TPoint;
//     using value_type = TValue;
//     using size_type = size_t;
//     std::map<TPoint, TValue> data;
//     template <typename Fn>
//     void query(const point_type &lower, const point_type &upper, Fn &&func)
//     {
//         auto [l1, l2, l3] = lower;
//         auto [u1, u2, u3] = upper;
//         for (auto [p, v] : data)
//         {
//             auto [p1, p2, p3] = p;
//             if (l1 <= p1 && l2 <= p2 && l3 <= p3 && p1 <= u1 && p2 <= u2 &&
//                 p3 <= u3)
//             {
//                 func(v);
//             }
//         }
//     }
//     [[nodiscard]] auto points() const noexcept
//     {
//         return data | std::views::values;
//     }
// };

// original orth
template <typename TPoint>
struct orthogonal_search_tree_node<TPoint, 2>
{
    // note dim x is unbounded
    using point_type = TPoint;
    // using size_type = size_t;
    static constexpr size_type dim_y = 1;
    static constexpr size_type dim_x = 0;
    using x_type = std::tuple_element_t<dim_x, TPoint>;
    using y_type = std::tuple_element_t<dim_y, TPoint>;
    using value_type = std::pair<TPoint, size_type>;
    [[nodiscard]] size_type num_node() const noexcept
    {
        return _data.size();
    }
    orthogonal_search_tree_node() = default;
    orthogonal_search_tree_node(std::vector<value_type> &&points) : _data(points.size())
    {
        build_heap(std::span{points}, std::span{_data});
    }
    void build_heap(std::span<value_type> points, tree_view<std::tuple<std::pair<x_type, y_type>, y_type, size_type>> tree)
    {
        if (tree.empty()) return;
        auto iter = std::ranges::max_element(points, {}, [](const auto &x) { return std::get<dim_x>(x.first); });
        x_type px = std::get<dim_x>(iter->first);
        y_type py = std::get<dim_y>(iter->first);
        size_type id = iter->second;
        std::swap(points.front(), *iter);
        auto sub_points = points.subspan(1);
        auto mid_iter = sub_points.begin() + sub_points.size() / 2;
        std::ranges::nth_element(sub_points, mid_iter, {}, [](const auto &x) { return std::make_pair(std::get<dim_y>(x.first),std::get<dim_x>(x.first)); });
        tree.root() = {std::pair{px, py}, std::get<dim_y>(mid_iter->first), id};
        auto split = (sub_points.size() + 1) / 2;
        build_heap(sub_points.subspan(0, split), tree.lower_child());
        build_heap(sub_points.subspan(split), tree.upper_child());
    }

    template <typename Fn>
    void query(const point_type &lower, const point_type &upper, Fn &&func)
    {
        auto lower_y = std::get<dim_y>(lower);
        auto upper_y = std::get<dim_y>(upper);
        auto lower_x = std::get<dim_x>(lower);
        auto tree = tree_view<std::tuple<std::pair<x_type, y_type>, y_type, size_type>>{std::span{_data}};
        while (!tree.empty())
        {
            auto [point, key, id] = tree.root();
            auto [px, py] = point;
            if(lower_x<=px && lower_y <=py && py<= upper_y) func(id);
            if (upper_y < key)
                tree = tree.lower_child();
            else if (lower_y > key)
                tree = tree.upper_child();
            else
                break;
        }
        if (tree.empty())
            return;
        // it's split node now
        auto lower_cur = tree.lower_child();
        while (!lower_cur.empty())
        {
            auto [point, key, id] = lower_cur.root();
            auto [px, py] = point;
            if (lower_x <= px && lower_y <= py && py <= upper_y) func(id);
            if (lower_y <= key)
            {
                report_in_subtree(lower_cur.upper_child(), lower_x, func);
                lower_cur = lower_cur.lower_child();
            }
            else
            {
                lower_cur = lower_cur.upper_child();
            }
        }
        auto upper_cur = tree.upper_child();
        while (!upper_cur.empty())
        {
            auto [point,key,id] = upper_cur.root();
            auto [px,py] = point;
            if (lower_x <= px && lower_y <= py && py <= upper_y)
                func(id);
            if (upper_y < key)
            {
                upper_cur = upper_cur.lower_child(); 
            }
            else
            {
                report_in_subtree(upper_cur.lower_child(), lower_x, func);
                upper_cur = upper_cur.upper_child();
            }
        }
    }
    template <typename Fn>
    void report_in_subtree(tree_view<std::tuple<std::pair<x_type, y_type>, y_type, size_type>> tree, const x_type &lower_x, Fn &&func)
    {
        if(tree.empty()) return;
        auto [point,key,id] = tree.root();
        // print("report {}",tree.root());
        auto [px,py] = point;
        if(lower_x<=px){
            func(id);
            report_in_subtree(tree.lower_child(),lower_x,func);
            report_in_subtree(tree.upper_child(),lower_x,func);
        }
    }
    std::vector<std::tuple<std::pair<x_type, y_type>, y_type, size_type>> _data;
};
