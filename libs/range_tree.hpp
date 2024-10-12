#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

template <typename TPoint, size_t Dimension = std::tuple_size_v<TPoint>>
struct range_tree_node
{
    using size_type = size_t;
    using key_type = std::tuple_element_t<Dimension - 1, TPoint>;
    using value_type = std::pair<TPoint, size_type>;
    using point_type = TPoint;
    std::unique_ptr<range_tree_node> lower_child;
    std::unique_ptr<range_tree_node> upper_child;
    range_tree_node<TPoint, Dimension - 1> next_dim;
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
    range_tree_node() = default;
    range_tree_node(std::vector<value_type> &&points)
    {
        assert(("fuck you", !points.empty()));
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
    range_tree_node(std::span<value_type> points,
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
                lower_child = std::make_unique<range_tree_node<TPoint, Dimension>>(points.subspan(0, mid_index), lower_indice, base_index);
            if (auto upper_indice = unique_indice.subspan(mid + 1);
                !upper_indice.empty())
                upper_child = std::make_unique<range_tree_node<TPoint, Dimension>>(points.subspan(mid_index), upper_indice, mid_index + base_index);
        }
        auto temp = std::vector<value_type>{};
        std::ranges::copy(points, std::back_inserter(temp));
        next_dim = range_tree_node<TPoint, Dimension - 1>{std::move(temp)};
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
        if (upper_key < key)
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
struct range_tree_node<TPoint, 1>
{
    using point_type = TPoint;
    using size_type = size_t;
    using key_type = std::tuple_element_t<0, TPoint>;
    using value_type = std::pair<TPoint, size_type>;
    [[nodiscard]] size_type num_node() const noexcept
    {
        return _data.size();
    }
    range_tree_node() = default;
    range_tree_node(std::vector<value_type> &&points)
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

template <typename TPoint, typename TValue = void>
class orthogonal_search_tree
{
    public:
    using point_type = TPoint;
    using value_type = TValue;
    using size_type = size_t;

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
        _root = std::make_unique<range_tree_node<TPoint>>(std::move(points));
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
    std::unique_ptr<range_tree_node<TPoint>> _root;
    std::vector<value_type> _data;
};

// FOR DEBUG
template <typename TPoint, typename TValue = void>
class range_debug_tree
{
    public:
    using point_type = TPoint;
    using value_type = TValue;
    using size_type = size_t;
    std::map<TPoint, TValue> data;
    template <typename Fn>
    void query(const point_type &lower, const point_type &upper, Fn &&func)
    {
        auto [l1, l2, l3] = lower;
        auto [u1, u2, u3] = upper;
        for (auto [p, v] : data)
        {
            auto [p1, p2, p3] = p;
            if (l1 <= p1 && l2 <= p2 && l3 <= p3 && p1 <= u1 && p2 <= u2 &&
                p3 <= u3)
            {
                func(v);
            }
        }
    }
    [[nodiscard]] auto points() const noexcept
    {
        return data | std::views::values;
    }
};
