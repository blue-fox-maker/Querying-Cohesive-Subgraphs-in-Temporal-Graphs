#include "func.hpp"
#include "seq.hpp"
#include <array>
#include <concepts>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

// implemention
template <typename TKey, typename TPriority, typename TValue, typename Compare = std::less<TPriority>>
class priority_search_tree
{
    public:
    using key_type = TKey;
    using priority_type = TPriority;
    using value_type = TValue;
    using point_type = std::pair<key_type, priority_type>;
    Compare cmp;
    struct node
    {
        key_type key;
        priority_type priority;
        value_type value;
        key_type split;
    };
    size_t size() const noexcept { return _nodes.size(); }
    priority_search_tree() = default;
    template <std::ranges::input_range R>
    priority_search_tree(R &&rng) : _nodes(std::ranges::size(rng))
    {
        auto data = std::vector<std::tuple<key_type, priority_type, value_type>>(std::ranges::begin(rng), std::ranges::end(rng));
        construct(std::span{data}, std::span{_nodes});
    }
    void construct(std::span<std::tuple<key_type, priority_type, value_type>> data, tree_view<node> tree)
    {
        if (tree.empty())
            return;
        auto iter = std::ranges::min_element(data, cmp, fn::get<1>);
        // auto iter = std::ranges::max_element(data, {}, [](const auto &x) { return std::get<1>(x); });
        tree.root().key = std::get<0>(*iter);
        tree.root().priority = std::get<1>(*iter);
        tree.root().value = std::get<2>(*iter);
        // *iter = std::move(data.front());
        std::swap(data.front(), *iter);
        auto sub_data = data.subspan(1);
        auto mid_iter = sub_data.begin() + sub_data.size() / 2;
        std::ranges::nth_element(sub_data, mid_iter, {}, fn::get<0>);
        tree.root().split = std::get<0>(*mid_iter);
        auto mid = (sub_data.size() + 1) / 2;
        construct(sub_data.subspan(0, mid), tree.lower_child());
        construct(sub_data.subspan(mid), tree.upper_child());
    }
    template <typename Fn>
    void _query(tree_view<node> tree, const key_type &lower_key, const key_type &upper_key, const priority_type &priority, Fn &&func)
    {
        if (tree.empty())
            return;
        auto &node = tree.root();
        if (cmp(priority, node.priority))
            return;
        if (lower_key <= node.key && node.key <= upper_key)
            func(node.value);
        if (lower_key <= node.split)
            _query(tree.lower_child(), lower_key, upper_key, priority, func);
        if (node.split <= upper_key)
            _query(tree.upper_child(), lower_key, upper_key, priority, func);
    }
    template <typename Fn>
    void query(const key_type &lower_key, const key_type &upper_key, const priority_type &priority, Fn &&func)
    {
        return _query(std::span{_nodes}, lower_key, upper_key, priority, std::forward<Fn>(func));
    }
    std::vector<node> _nodes;
};

// a<=x<=b<=y
template <typename TKey, typename TValue>
class AXBY_query
{
    public:
    using key_type = TKey;
    using point_type = std::array<key_type, 2>;
    using value_type = TValue;
    size_t num_point() const noexcept { return search_tree.size(); }
    priority_search_tree<TKey, TKey, TValue, std::greater<void>> search_tree;
    AXBY_query() = default;
    template <std::ranges::input_range R>
    AXBY_query(R &&rng) : search_tree(rng | std::views::transform([](auto &&x) { return std::make_tuple(std::get<0>(x.first), std::get<1>(x.first), x.second); })) {}
    template <typename Fn>
    void query(const key_type &a, const key_type &b, Fn &&func)
    {
        return search_tree.query(a, b, b, std::forward<Fn>(func));
    }
};
// a<=x<=y<=b
template <typename TKey, typename TValue>
class AXYB_query
{
    public:
    using key_type = TKey;
    using point_type = std::array<key_type, 2>;
    using value_type = TValue;
    size_t num_point() const noexcept { return search_tree.size(); }
    priority_search_tree<TKey, TKey, TValue, std::less<void>> search_tree;
    AXYB_query() = default;
    template <std::ranges::input_range R>
    AXYB_query(R &&rng) : search_tree(rng | std::views::transform([](auto &&x) { return std::make_tuple(std::get<0>(x.first), std::get<1>(x.first), x.second); })) {}
    template <typename Fn>
    void query(const key_type &a, const key_type &b, Fn &&func)
    {
        return search_tree.query(a, b, b, std::forward<Fn>(func));
    }
};

// a<=x<=y<=b<=z
template <typename TKey, typename TValue>
class AXYBZ_query
{
    public:
    using key_type = TKey;
    using value_type = TValue;
    using point_type = std::array<TKey, 3>;
    struct node
    {
        node(std::span<std::pair<point_type, value_type>> data)
        {
            split = kth_element(data | std::views::keys | std::views::elements<1>, data | std::views::keys | std::views::elements<2>, data.size());
            std::vector<std::pair<point_type, value_type>> lower_data;
            std::vector<std::pair<point_type, value_type>> upper_data;
            std::vector<std::pair<typename decltype(axyb)::point_type, value_type>> axyb_data;
            std::vector<std::pair<typename decltype(axbz)::point_type, value_type>> axbz_data;
            for (auto &&[point, val] : std::move(data))
            {
                auto &&[x, y, z] = point;
                if (z < split)
                    lower_data.emplace_back(std::move(point), std::move(val));
                else if (y > split)
                    upper_data.emplace_back(std::move(point), std::move(val));
                else
                {
                    axyb_data.emplace_back(std::array{x, y}, val);
                    if (z != split)
                        axbz_data.emplace_back(std::array{x, z}, val);
                }
            }
            axyb = std::move(axyb_data);
            axbz = std::move(axbz_data);
            if (!lower_data.empty())
                lower_child_ptr = std::make_unique<node>(lower_data);
            if (!upper_data.empty())
                upper_child_ptr = std::make_unique<node>(upper_data);
        }
        std::unique_ptr<node> lower_child_ptr;
        std::unique_ptr<node> upper_child_ptr;
        key_type split;
        AXYB_query<TKey, TValue> axyb;
        AXBY_query<TKey, TValue> axbz;
        size_t num_node() const
        {
            return 1 + (lower_child_ptr ? lower_child_ptr->num_node() : 0) + (upper_child_ptr ? upper_child_ptr->num_node() : 0);
        }
    };
    template <std::ranges::input_range R>
    AXYBZ_query(R &&rng)
    {
        auto data = std::vector<std::pair<point_type, value_type>>(std::ranges::begin(std::forward<R>(rng)), std::ranges::end(std::forward<R>(rng)));
        _root = std::make_unique<node>(std::span{data});
    }
    std::unique_ptr<node> _root;
    template <typename Fn>
    void query(const key_type &a, const key_type &b, Fn &&func)
    {
        auto tree = _root.get();
        while (tree)
        {
            if (b <= tree->split) // BZ
            {
                // AXYB
                tree->axyb.query(a, b, func);
            }
            else if (b > tree->split) // YB
            {
                // AXBZ
                tree->axbz.query(a, b, func);
            }
            if (b < tree->split)
                tree = tree->lower_child_ptr.get();
            else if (b > tree->split)
                tree = tree->upper_child_ptr.get();
            else
                break;
        }
    }
    size_t num_node() const
    {
        return _root ? _root->num_node() : 0;
    }
};
