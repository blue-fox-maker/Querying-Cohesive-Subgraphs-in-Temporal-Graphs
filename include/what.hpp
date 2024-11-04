#pragma once
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>

#include "console.hpp"
#include "orthogonal.hpp"
#include "temporal_core.hpp"
#include "graph.hpp"

inline auto test_cc(std::filesystem::path file, size_t num_query, const std::vector<float>& ratios){
    auto ifs = std::ifstream{file};
    size_t num_time, num_vert;
    std::vector<std::tuple<size_t, size_t, int>> raw_data;
    ifs >> num_time >> num_vert >> raw_data;
    ::print("load graph with {} verts and {} edges in [0, {})", num_vert, raw_data.size(), num_time);
    auto index = bench([&]{
        auto tsf = saisho_zenyu_mori<int>{num_vert};
        auto points = std::map<std::array<int,2>, std::vector<edge<>>>{};
        for(auto [u,v,t]:raw_data){
            if(auto i = tsf.add_edge(u,v,t);i){
                auto e = tsf.edges()[*i];
                points[{e.attr,t-1}].emplace_back(e.source,e.target);
            }
        }
        for(auto [u,v,t]:tsf.edges())
            points[{t,std::numeric_limits<int>::max()}].emplace_back(u,v);
        return ekkusubiwai<int,std::vector<edge<>>>{points};
    }, "construct index");

    for(auto ratio: ratios){
      ::print("[ratio = {}]", ratio);
      auto queries = std::vector<std::pair<int,int>>(num_query);
      auto random_engine = std::default_random_engine(std::random_device{}());
      size_t length = ratio * num_time;
      auto distrib = std::uniform_int_distribution<size_t>(0, num_time - length - 2);
      std::ranges::generate(queries, [=, &random_engine, &distrib] {
          auto ts = distrib(random_engine);
          return std::make_pair(ts, ts + length);
      });
      auto res = std::vector<size_t>(num_query);
      bench([&] {
          for (size_t i = 0; i < queries.size(); i++)
              index.query(queries[i].first, queries[i].second, [&](const auto &x) {
                  for (auto v : x)
                      res[i]++;
              });
      }, "solve query");
    }
}

inline auto test_core(std::filesystem::path file ,size_t num_query, const std::vector<float> &ratios, const std::vector<float> &k_ratios){
    auto ifs = std::ifstream{file};
    size_t num_time, num_vert;
    std::vector<std::tuple<size_t, size_t, int>> raw_data;
    ifs >> num_time >> num_vert >> raw_data;
    std::ranges::sort(raw_data);
    ::print("load graph with {} verts and {} edges in [0, {})", num_vert, raw_data.size(), num_time);
    auto phc = Graph{file.string()};
    auto indexes = bench([&]{
        phc.index();
        auto index = std::vector<ekkusuwaibizetto<int,std::vector<size_t>>>{};
        for(size_t k = 2; k < phc.k_max_; k++)
        {
          auto points = std::map<std::array<int, 3>, std::vector<size_t>>{};
          for (size_t i = 1; i < num_vert; i++)
          {
              if (k < phc.core_t_[i].size())
                  for (size_t j = 0; j < phc.core_t_[i][k].size() - 1; j++)
                      points[{phc.core_t_[i][k][j + 1].first - 1, phc.core_t_[i][k][j].second, phc.core_t_[i][k][j + 1].second - 1}].push_back(i);
          }
          index.push_back(std::move(points)|std::views::all);
        }
        return index;
    }, "construct index");
    for(auto k_ratio: k_ratios){
      size_t k = phc.k_max_*k_ratio;
      ::print("[k = {}]", k);
      auto& index = indexes[k-2];
      for (auto ratio : ratios)
      {
          print("[ratio = {}]", ratio);
          auto queries = std::vector<std::pair<size_t, size_t>>(num_query);
          auto random_engine = std::default_random_engine(std::random_device{}());
          size_t length = ratio * num_time;
          auto distrib = std::uniform_int_distribution<size_t>(0, num_time - length - 2);
          std::ranges::generate(queries, [=, &random_engine, &distrib] {
              auto ts = distrib(random_engine);
              return std::make_pair(ts, ts + length);
          });
          auto res = std::vector<size_t>(num_query);
          bench([&] {
              for (size_t i = 0; i < queries.size(); i++)
                  index.query(queries[i].first, queries[i].second, [&](const auto &x) {
                      for (auto v : x)
                          res[i]++;
                  });
          }, "solve query");
       }
    }
}
