#pragma once
#include <algorithm>
#include <array>
#include <memory>
#include <ranges>
#include <tuple>
#include <vector>
#include "seq.hpp"
#include "func.hpp"
#include "index.hpp"

template <typename T, typename P, typename U, typename Cmp = std::less<void>>
class indekkusu_sutorakucha
{
    public:
    using X = std::pair<T, P>;
    struct node
    {
        T key;
        P pri;
        U value;
        T split;
    };
    size_t size() const noexcept { return _nodes.size(); }
    indekkusu_sutorakucha() = default;
    template <std::ranges::input_range R>
    indekkusu_sutorakucha(R &&rng) : _nodes(std::ranges::size(rng))
    {
        auto data = std::vector<std::tuple<T, P, U>>(std::ranges::begin(rng), std::ranges::end(rng));
        construct(std::span{data}, std::span{_nodes});
    }
    void construct(std::span<std::tuple<T, P, U>> data, tree_view<node> tree)
    {
        if (tree.empty())
            return;
        auto iter = std::ranges::min_element(data, Cmp{}, fn::get<1>);
        tree.root().key = std::get<0>(*iter);
        tree.root().pri = std::get<1>(*iter);
        tree.root().value = std::get<2>(*iter);
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
    void _query(tree_view<node> tree, const T &lower_key, const T &upper_key, const P &priority, Fn &&func)
    {
        if (tree.empty())
            return;
        auto &node = tree.root();
        if (Cmp()(priority, node.pri))
            return;
        if (lower_key <= node.key && node.key <= upper_key)
            func(node.value);
        if (lower_key <= node.split)
            _query(tree.lower_child(), lower_key, upper_key, priority, func);
        if (node.split <= upper_key)
            _query(tree.upper_child(), lower_key, upper_key, priority, func);
    }
    template <typename Fn>
    void query(const T &lower_key, const T &upper_key, const P &priority, Fn &&func)
    {
        return _query(std::span{_nodes}, lower_key, upper_key, priority, std::forward<Fn>(func));
    }
    auto data() const
    {
        return _nodes | std::views::transform([](const auto &x) {return std::make_pair(std::make_pair(x.key, x.pri), x.value); });
    }
    std::vector<node> _nodes;
};

template <typename T, typename U>
class ekkusubiwai
{
    public:
    using P = std::array<T, 2>;
    size_t num_point() const noexcept { return search_tree.size(); }
    indekkusu_sutorakucha<T, T, U, std::greater<void>> search_tree;
    ekkusubiwai() = default;

    ekkusubiwai(auto &&rng) : search_tree(rng | std::views::transform([](auto &&x) { return std::make_tuple(std::get<0>(x.first), std::get<1>(x.first), x.second); })) {}

    void query(const T &a, const T &b, const auto &func)
    {
        return search_tree.query(a, b, b, func);
    }
    auto data()const{ return search_tree.data();}
};

template <typename T, typename U>
class ekkusuwaibi
{
    public:
    using P = std::array<T, 2>;
    size_t num_point() const noexcept { return search_tree.size(); }
    indekkusu_sutorakucha<T, T, U, std::less<void>> search_tree;
    ekkusuwaibi() = default;
    template <std::ranges::input_range R>
    ekkusuwaibi(R &&rng) : search_tree(rng | std::views::transform([](auto &&x) { return std::make_tuple(std::get<0>(x.first), std::get<1>(x.first), x.second); })) {}
    template <typename Fn>
    void query(const T &a, const T &b, Fn &&func)
    {
        return search_tree.query(a, b, b, std::forward<Fn>(func));
    }
    auto data()const{ return search_tree.data();}
};

template <typename T, typename U>
class ekkusuwaibizetto
{
    public:
    using P = std::array<T, 3>;
    struct node
    {
        node(std::span<std::pair<P, U>> data)
        {
            split = kth_element(data | std::views::keys | std::views::elements<1>, data | std::views::keys | std::views::elements<2>, data.size());
            std::vector<std::pair<P, U>> lower_data;
            std::vector<std::pair<P, U>> upper_data;
            std::vector<std::pair<typename decltype(axyb)::P, U>> axyb_data;
            std::vector<std::pair<typename decltype(axbz)::P, U>> axbz_data;
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
        T split;
        ekkusuwaibi<T, U> axyb;
        ekkusubiwai<T, U> axbz;
        size_t num_node() const
        {
            return 1 + (lower_child_ptr ? lower_child_ptr->num_node() : 0) + (upper_child_ptr ? upper_child_ptr->num_node() : 0);
        }
        void insert(const P &p, const U &val)
        {
            auto &&[x, y, z] = p;
            if (z < split)
                if (lower_child_ptr)
                    lower_child_ptr->insert(p, val);
                else
                    lower_child_ptr = std::make_unique<node>(std::span{std::vector{val}});
            else if (y > split)
                if (upper_child_ptr)
                    upper_child_ptr->insert(p, val);
                else
                    upper_child_ptr = std::make_unique<node>(std::span{std::vector{val}});
            else
            {
                axyb.insert(std::array{x, y}, val);
                if (z != split)
                    axbz.insert(std::array{x, z}, val);
            }
        }
    };
    ekkusuwaibizetto(auto &&rng)
    {
        auto data = std::vector<std::pair<P, U>>(std::ranges::begin(rng), std::ranges::end(rng));
        _root = std::make_unique<node>(std::span{data});
    }
    void insert(const P &p, const U &val)
    {
        assert(_root);
        _root->insert(p, val);
    }
    std::unique_ptr<node> _root;
    void query(const T &a, const T &b, auto &&func)
    {
        auto tree = _root.get();
        while (tree)
        {
            if (b <= tree->split)
                tree->axyb.query(a, b, func);
            else if (b > tree->split)
                tree->axbz.query(a, b, func);
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
