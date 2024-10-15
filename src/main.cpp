#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>

#include "../include/argparse.hpp"
#include "../include/console.hpp"
#include "../include/orthogonal.hpp"
#include "../include/star.hpp"

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
    auto edges = std::map<std::pair<size_t,size_t>,std::vector<int>>{};
    for(auto [u,v,t]:raw_data)
        edges[{u,v}].push_back(t);
    auto iter = std::unique(raw_data.begin(), raw_data.end());
    raw_data.erase(std::unique(raw_data.begin(), raw_data.end()), raw_data.end());
}
