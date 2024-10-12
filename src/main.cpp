#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>

#include "../libs/baseline.hpp"
#include "../libs/config.hpp"
#include "../libs/console.hpp"
#include "../libs/index.hpp"
#include "../libs/orthogonal.hpp"
// #include "../libs/priority_search_tree.hpp"
// #include "../libs/segment_tree.hpp"

int main(int argc, char *argv[])
{
    using namespace std::literals;
    const auto dataset_file = std::filesystem::path{argv[1]};
    const size_t num_query = std::stoi(argv[2]);
    print("load data from {} and test with {} queries", dataset_file, num_query);
    print("---------------------------------------------------------------------------");
    auto ifs = std::ifstream{dataset_file};
    size_type num_time = 0;
    size_type num_vert = 0;
    std::vector<std::tuple<size_type, size_type, size_type>> raw_data;
    std::vector<std::tuple<size_type, size_type, size_type, size_type>>
        temporal_edges;
    ifs >> num_time >> num_vert >> raw_data;
    std::ranges::transform(
        std::move(raw_data), std::back_inserter(temporal_edges),
        [](const std::tuple<size_type, size_type, size_type> &x) {
            auto [u, v, t] = x;
            return std::make_tuple(u, v, t, t);
        });
    print("load graph with {} verts and {} edges in [0, {})", num_vert,
          raw_data.size(), num_time);
    auto index = bench([&] {
        auto data = std::map<std::pair<size_type, size_type>, std::vector<std::pair<size_type, size_type>>>{};
        auto forest = temporal_spanning_forest(num_time, num_vert, std::move(temporal_edges));
        print("rank is {}", forest.size());
        for (auto &&[k, v] : forest)
        {
            auto [t1, t2, t3] = k;
            assert(t1 == t2);
            assert(t1 <= t3);
            data[{t1, t3}] = v;
        }
        auto temp = std::vector<std::tuple<size_type, size_type, std::vector<std::pair<size_type, size_type>>>>{};
        for (auto &&[k, v] : data)
            temp.push_back(std::tuple{k.first, k.second, v});
        print("construct");
        return AXBY_query<size_type, std::vector<std::pair<size_type, size_type>>>{data};
    },"compute temporal spanning forest");

    auto baseline_index = bench([&] {
        auto data = std::map<std::pair<size_type, size_type>, std::vector<std::pair<size_type, size_type>>>{};
        for (auto &&[k, v] : temporal_spanning_forest(num_time, num_vert, std::move(temporal_edges)))
        {
            auto [t1, t2, t3] = k;
            assert(t1 == t2);
            data[{t1, t3}] = v;
        }
        return baseline<std::vector<std::pair<size_type, size_type>>>{static_cast<size_t>(num_time), data};
    },"construct baseline index");

    for (auto ratio : {0.2, 0.4, 0.6, 0.8})
    {
        print("[[ratio = {}]]", ratio);
        auto queries = std::vector<std::pair<size_type, size_type>>(num_query);
        auto random_engine = std::default_random_engine(std::random_device{}());
        size_type length = ratio * num_time;
        auto distrib = std::uniform_int_distribution<size_type>(0, num_time - length - 1);
        std::ranges::generate(queries, [=, &random_engine, &distrib] {
            auto ts = distrib(random_engine);
            return std::make_pair(ts, ts + length);
        });
        std::vector<size_type> result1(num_query);
        std::vector<size_type> result2(num_query);

        bench([&] {
            for (size_t i = 0; i < num_query; i++)
            {
                auto [ts, te] = queries[i];
                index.query(ts, te, [&](const auto &point) {
                    for (auto &&val : point)
                        result1[i]++;
                });
            }
        },"solve query");
        bench( [&] {
            for (size_t i = 0; i < num_query; i++)
            {
                auto [ts, te] = queries[i];
                baseline_index.solve(ts, te, [&](const auto &point) {
                    for (auto &&val : point)
                        result2[i]++;
                });
            }
        },"baseline query");
        print("result1 = result2 ? {}", result1 == result2);
    }
}
