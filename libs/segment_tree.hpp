#include "seq.hpp"
#include <bit>
#include <cstdio>

template <typename T>
class interval_segement_tree
{
    public:
    using size_type = size_t;
    using interval_type = std::pair<size_type, size_type>;
    using value_type = T;
    template <typename R>
    interval_segement_tree(size_t, R &&);
    void insert(const interval_type &, size_type, size_type = 0);
    template <typename Fn>
    void query(const interval_type&, Fn &&func);
    template <typename Fn>
    void _query(const interval_type &, Fn &&func, size_type = 0);
    interval_type range(size_type i)
    {
        i++;
        auto x = std::bit_width(i) - 1;
        auto y = i - std::bit_floor(i);
        auto rank = 1 << (std::bit_width(data.size()) - x - 1);
        return {y * rank, (y + 1) * rank - 1};
    }
    size_type capacity() const { return (data.size()+1)/2;}
    size_type count() const {return _count;}

    // private:
    std::vector<std::vector<std::pair<size_type,size_type>>> data;
    std::vector<value_type> vals;
    size_t _count;
};

template <typename T>
template <typename R>
interval_segement_tree<T>::interval_segement_tree(size_t n, R &&rng) : data(std::bit_ceil(n) * 2 -1)
{
    // std::ranges::sort(rng);
    for (auto &&[interval,val] : rng)
    {
        insert(interval,vals.size());
        vals.push_back(val);
    }
    for (auto &val:data){
        std::ranges::sort(val);
    }
}

template <typename T>
void interval_segement_tree<T>::insert(const interval_type &interval,size_type idx, size_type root)
{
    auto cur = range(root);
    if (interval.first <= cur.first && cur.second <= interval.second)
    {
        data[root].emplace_back(interval.first,idx);
        _count++;
    }
    else
    {
        auto mid = (interval.first + interval.second) / 2;
        for (auto child : {2 * root + 1, 2 * root + 2})
        {
            auto child_rng = range(child);
            if (std::max(interval.first, child_rng.first) <= std::min(interval.second, child_rng.second))
                insert(interval,idx, child);
        }
    }
}

template <typename T>
template <typename Fn>
void interval_segement_tree<T>::query(const interval_type &interval, Fn &&func)
{
    _query(interval,func,0);
}

template <typename T>
template <typename Fn>
void interval_segement_tree<T>::_query(const interval_type &interval, Fn &&func, size_type root)
{
    for (auto &&[ts, idx] : data[root]|std::views::reverse)
    {
        if (ts < interval.first)
            break;
        func(vals[idx]);
    }
    if (auto cur = range(root);cur.first==cur.second)
        return;
    if (auto lower_rng = range(2 * root + 1); interval.second <= lower_rng.second)
        _query(interval, func, 2 * root + 1);
    else
        _query(interval, func, 2 * root + 2);
}
