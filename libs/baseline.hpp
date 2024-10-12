#include "config.hpp"
#include "console.hpp"
#include <limits>
#include <vector>

template <typename T>
class baseline
{
    public:
    using size_type = int;
    private:
    std::vector<std::vector<T>> data;
    std::vector<std::vector<size_type>> actual_time;

    size_type find_an_index(size_type t, size_type ts, size_type te)
    {
        int l = 0;
        int r = actual_time[t].size() - 1;

        if (r == -1 || actual_time[t][r] < ts || actual_time[t][0] > te)
        {
            return -1;
        }

        while (l < r)
        {
            int mid = (l + r) >> 1;
            if (actual_time[t][mid] >= ts && actual_time[t][mid] <= te)
            {
                return mid;
            }
            else
            {
                if (actual_time[t][mid] < ts)
                {
                    l = mid + 1;
                }
                else
                {
                    r = mid;
                }
            }
        }

        if (actual_time[t][l] >= ts && actual_time[t][l] <= te)
        {
            return l;
        }
        else
        {
            return -1;
        }
    }

    public:
    template <typename R>
    baseline(size_t num_time, R &&rng) : data(num_time+1), actual_time(num_time+1)
    {
        for (auto &&[key, val] : rng)
        {
            auto [ts, te] = key;
            if(te == std::numeric_limits<decltype(te)>::max()) te = num_time;
            data[te].push_back(val);
            actual_time[te].push_back(ts);
        }
    }

    template <typename Fn>
    auto solve(size_type ts, size_type te, Fn &&func)
    {
        for (size_t t = te; t < actual_time.size(); t++)
        {
            auto idx = find_an_index(t, ts, te);
            if (idx == -1)
                continue;
            for (size_type i = idx; i >= 0; i--)
            {
                if (actual_time[t][i] < ts)
                    break;
                func(data[t][i]);
            }
            for (size_type i = idx + 1; i < actual_time[t].size(); i++)
            {
                if (actual_time[t][i] > te)
                    break;
                func(data[t][i]);
            }
        }
    }
};
