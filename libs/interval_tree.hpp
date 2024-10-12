#include "console.hpp"
#include <bits/ranges_algo.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>

template <typename T, typename U>
class half_interval_tree
{
    public:
    using mapped_type = U;
    using key_type = T;
    using interval_type = std::pair<T, T>;
    using size_type = size_t;

    private:
    struct node
    {
        std::unique_ptr<node> lower_child;
        std::unique_ptr<node> upper_child;
        key_type key;
        std::vector<std::pair<interval_type, size_type>> val;
        node(std::span<std::pair<interval_type, size_type>> data);
    };

    std::unique_ptr<node> root;
    std::vector<mapped_type> data;

    public:
    template <std::ranges::input_range R>
    half_interval_tree(R &&);

    // query intervals [a,b] with lower<=a<=upper<=b
    template <typename Fn>
    void query(key_type lower, key_type upper, Fn &&func);
};

template <typename T, typename U>
template <std::ranges::input_range R>
half_interval_tree<T, U>::half_interval_tree(R &&rng) : data(std::ranges::size(rng))
{
    std::vector<std::pair<interval_type, size_type>> points(std::ranges::size(rng));
    size_t count = 0;
    for (auto &&[interval, val] : std::forward<R>(rng))
    {
        count++;
        data[count] = std::forward<decltype(val)>(val);
        points[count] = std::pair{std::forward<decltype(interval)>(interval), count};
    }
    std::ranges::sort(points);
    std::ranges::sort(data, {}, [](const auto &x) { return x.first.second; });
    root = std::make_unique<node>(std::span{points});
}

template <typename T, typename U>
template <typename Fn>
void half_interval_tree<T, U>::query(key_type lower, key_type upper, Fn &&func)
{
    auto cur = root;
    while (cur)
    {
        if (upper < cur->key)
        {
            for (auto &&[interval, idx] : cur->val | std::views::reverse)
            {
                if (interval.first <= lower)
                    break;
                func(data[idx]);
            }
            cur = cur->lower_child;
        }
        else if (upper > cur->key)
        {
            cur = cur->upper_child;
        }
        else
            return;
    }
}

template <typename T, typename U>
half_interval_tree<T, U>::node::node(std::span<std::pair<interval_type, size_type>> data)
{
    assert(!data.empty());
    key = data[data.size() / 2].first.second;
    auto [iter_lower, iter_upper] = std::ranges::equal_range(data, key, {}, [](const auto &x) { return x.first.second; });
    const auto lower_data = std::span{data.begin(), iter_lower};
    const auto upper_data = std::span{iter_upper, data.end()};
    std::ranges::copy(std::span{iter_lower, iter_upper}, std::back_inserter(val));
    std::ranges::copy(upper_data | std::views::filter([this](const auto &x) { return x.first.first <= key && key <= x.first.second; }), std::back_inserter(val));
    if (!lower_data.empty())
        lower_child = std::make_unique<node>(lower_data);
    if (!upper_data.empty())
        upper_child = std::make_unique<node>(upper_data);
}
