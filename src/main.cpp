#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <random>

#include "../include/argparse.hpp"
#include "../include/console.hpp"
// #include "../include/orthogonal.hpp"
#include "../include/star.hpp"

int main(int argc, char *argv[])
{
    auto args = argparse::ArgumentParser{};
    args.add_argument("file").help("dataset path");
    args.add_argument("-n", "--num_query").help("number of test queries").scan<'i', size_t>().default_value(10000);
    args.parse_args(argc, argv);
    const auto file = std::filesystem::path{args.get<std::string>("file")};
    const auto num_query = args.get<size_t>("--num_query");

    print("running on dataset {} and test with {} queries", file, num_query);
    auto ifs = std::ifstream{file};
    size_t num_time, num_vert;
    std::vector<std::tuple<size_t, size_t, int>> raw_data;
    ifs >> num_time >> num_vert >> raw_data;
    auto edges = std::map<std::pair<size_t,size_t>,std::vector<int>>{};
    for(auto [u,v,t]:raw_data)
        edges[{u,v}].push_back(t);
    auto temporal_edges = std::vector<edge<std::vector<int>>>(edges.size());
    for(auto&& [uv,t]:edges)
        temporal_edges.emplace_back(uv.first,uv.second,std::move(t));

    auto G = graph{num_vert, temporal_edges};
    auto msf = minimum_spanning_forest<int>{num_vert};
    auto points = std::map<std::array<int,2>, std::vector<edge<>>>{};
    for(auto [u,v,t]:raw_data){
        if(auto i = msf.add_edge(u,v,t);i){
            auto e = msf.edges()[*i];
            points[{e.attr,t-1}].emplace_back(e.source,e.target);
        }
    }
    for(auto [u,v,t]:msf.edges())
        points[{t,std::numeric_limits<int>::max()}].emplace_back(u,v);
    // auto index = AXBY_query<int,std::vector<edge<>>>{points};

    auto queries = std::vector<std::pair<int,int>>{};
    bench([&]{
        for(auto [ts,te]: queries)
            index.query(ts,te);
    },"solve query");
}
