#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>

#include "../libs/argparse.hpp"
#include "../libs/baseline.hpp"
#include "../libs/console.hpp"
#include "../libs/graph.hpp"
#include "../libs/orthogonal.hpp"

int main(int argc, char *argv[])
{
    auto args = argparse::ArgumentParser{};
    args.add_argument("file").help("dataset path");
    args.add_argument("-n", "--num_query").help("number of test queries").scan<'i', size_t>().default_value(10000);
    args.parse_args(argc, argv);
    auto file = std::filesystem::path{args.get<std::string>("file")};
    auto num_query = args.get<size_t>("--num_query");

    print("running on dataset {} and test with {} queries", file, num_query);
    auto ifs = std::ifstream{file};
    size_t num_time, num_vert;
    std::vector<std::tuple<size_t, size_t, int>> raw_data;
    ifs >> num_time >> num_vert >> raw_data;
    std::ranges::sort(raw_data);
    auto iter = std::unique(raw_data.begin(), raw_data.end());
    raw_data.erase(std::unique(raw_data.begin(), raw_data.end()), raw_data.end());
    std::vector<edge_descriptor<size_t, int>> edges;
    for (auto [u, v, t] : raw_data)
        edges.emplace_back(u, v, t);

    auto points = bench([&] {
        return TSF_index(edges, [](auto t) { return std::pair{t, t}; });
    },"construct TSF");
    size_t rank = 0;
    auto points_2d = std::map<std::array<int, 2>, std::vector<std::pair<int, int>>>{};
    for (auto [p, v] : points)
    {
        rank += v.size();
        auto [x, y, z] = p;
        assert(x == y);
        for (auto e : v)
        {
            points_2d[{x, z}].emplace_back(e.source_id, e.target_id);
        }
    }
    print("the rank is {}", rank);
    auto index = bench([&] {
        return AXBY_query<int, std::vector<std::pair<int, int>>>{points_2d};
    },"construct index");
    auto baseline_index = baseline<std::vector<std::pair<int, int>>>{static_cast<size_t>(num_time), points_2d};

    for (auto ratio : {0.2, 0.4, 0.6, 0.8})
    {
        print("[[ratio = {}]]", ratio);
        auto queries = std::vector<std::pair<int, int>>(num_query);
        auto random_engine = std::default_random_engine(std::random_device{}());
        int length = ratio * num_time;
        auto distrib = std::uniform_int_distribution<int>(0, num_time - length - 1);
        std::ranges::generate(queries, [=, &random_engine, &distrib] {
            auto ts = distrib(random_engine);
            return std::make_pair(ts, ts + length);
        });
        std::vector<size_type> result1(num_query);
        std::vector<size_type> result2(num_query);
        std::vector<size_type> result3(num_query);

        bench([&] {
            for (size_t i = 0; i < num_query; i++)
            {
                auto [ts, te] = queries[i];
                index.query(ts, te, [&](const auto &point) {
                    for (auto &&val : point)
                        result1[i]++;
                });
            }
        },
              "solve query");
        bench([&] {
            for (size_t i = 0; i < num_query; i++)
            {
                auto [ts, te] = queries[i];
                baseline_index.solve(ts, te, [&](const auto &point) {
                    for (auto &&val : point)
                        result2[i]++;
                });
            }
        },"baseline query");
        if (result1 != result2)
        {
            for (size_t i = 0; i < num_query; i++)
            {
                if (result1[i] != result2[i])
                {
                    print("result 1 = {}, result 2 = {}, at {}", result1[i], result2[i], queries[i]);
                }
            }
        }
    }
}
