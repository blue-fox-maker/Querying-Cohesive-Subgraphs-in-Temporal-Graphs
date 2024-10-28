#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <random>
#include <cassert>
#include <ranges>
#include <stdexcept>
#include "../include/argparse.hpp"
#include "../include/console.hpp"
#include "../include/orthogonal.hpp"
#include "../include/graph.hpp"
#include "../include/what.hpp"

int main(int argc, char *argv[])
{
    auto args = argparse::ArgumentParser{};
    args.add_argument("model").help("cc or core");
    args.add_argument("file").help("dataset path");
    args.add_argument("-n", "--num_query").help("number of test queries").scan<'i', size_t>().default_value(static_cast<size_t>(10000));
    args.add_argument("-r", "--ratio").help("ratio for query interval size").scan<'g',float>().nargs(argparse::nargs_pattern::at_least_one);
    args.add_argument("-k", "--k_ratio").help("ratio for k").scan<'g',float>().nargs(argparse::nargs_pattern::at_least_one);
    args.parse_args(argc, argv);
    const auto model = args.get<std::string>("model");
    const auto file = std::filesystem::path{args.get<std::string>("file")};
    const auto num_query = args.get<size_t>("--num_query");
    const auto ratios = args.get<std::vector<float>>("--ratio");

    if (model == "cc") test_cc(file,num_query,ratios);
    else if (model == "core") test_core(file, num_query, ratios, args.get<std::vector<float>>("k_ratio"));
    else throw std::invalid_argument(std::format("csm model {} is not supported",model));
}
