#pragma once
#include <algorithm>
#include <cassert>
#include <concepts>
#include <iostream>
#include <cstddef>
#include "console.hpp"
#include <iterator>
#include <limits>
#include <map>
#include <queue>
#include <ranges>
#include <concepts>
#include <type_traits>
#include <vector>
#include <numeric>

struct none {
  friend auto operator<=>(none,none)=default;
};

template <typename T = none> struct target_edge {
  using attr_t = T;
  size_t target;
  [[no_unique_address]] T attr;
  target_edge(size_t v, auto &&...args):target(v),attr(std::forward<decltype(args)>(args)...){}
  target_edge()=default;
  auto friend operator<=>(const target_edge& lhs,const target_edge& rhs){ return lhs.target<=>rhs.target;};
};
template <typename T = none> struct source_edge {
  using attr_t = T;
  size_t source;
  [[no_unique_address]] T attr;
  source_edge(size_t v, auto &&...args):source(v),attr(std::forward<decltype(args)>(args)...){}
  source_edge()=default;
  auto friend operator<=>(const source_edge& lhs,const source_edge& rhs){ return lhs.source<=>rhs.source; };
};
template <typename T = none> struct edge {
  using attr_t = T;
  size_t source;
  size_t target;
  T attr;
  edge(size_t u, size_t v, auto &&...args):source(u),target(v),attr(std::forward<decltype(args)>(args)...){}
  edge(size_t u, const source_edge<T>& x):source(x.source),target(u),attr(x.attr){}
  edge(size_t u, const target_edge<T>& x):source(u),target(x.target),attr(x.attr){}
  edge()=default;
  auto friend operator<=>(const edge& lhs,const edge& rhs){ return std::pair{lhs.source,rhs.target}<=>std::pair{lhs.source,rhs.target};};
};
template <typename T>
edge(size_t, size_t, T&&)->edge<T>;

template <typename EA, typename WProj = std::identity> 
class saisho_zenyu_mori {
  std::vector<target_edge<EA>> _data;
  std::vector<size_t> _depth; // this array is useless.
public:
  size_t num_vert() const noexcept { return _data.size(); }
  auto edges() const noexcept { return std::views::iota(0U,num_vert())|std::views::transform([this](size_t i){return edge{i,_data[i].target,_data[i].attr};});}
  template <std::ranges::range R = std::ranges::empty_view<edge<EA>>>
  saisho_zenyu_mori(size_t num_vert, R&& rng = {}):_data(num_vert),_depth(num_vert,0){
    for(size_t i = 0; i < _data.size(); i++ ) _data[i] = target_edge<EA>{i};
    for(auto&& e:rng) add_edge(e.source,e.target,e.attr);
  }
  [[maybe_unused]] std::optional<size_t> add_edge(size_t u, size_t v, auto &&...attr_args) {
    if (u==v) return std::nullopt;
    auto w = critical_edge(u,v);
    if (_depth[u] >_depth[v]) std::swap(u,v);
    if (w){
      if (WProj{}(_data[*w].attr) < WProj{}(_data[*w].attr)) return std::nullopt;
      _data[*w].target = *w;
    }
    reroot(u);
    _data[u]=target_edge<EA>{v,std::forward<decltype(attr_args)>(attr_args)...};
    return w;
  }

private:
  size_t find(size_t v) {
    if (auto p = _data[v].target; p != v) {
      auto res = find(p);
      _depth[v] = _depth[p] + 1;
      return res;
    }
    _depth[v] = 0;
    return v;
  }

  std::optional<size_t> critical_edge(size_t u, size_t v) {
    if (find(u) != find(v))
      return {};
    std::optional<size_t> cur;
    while (u != v) {
      if (_depth[u] < _depth[v])
        std::swap(u,v);
      if (!cur || WProj{}(_data[*cur].attr) < WProj{}(_data[*cur].attr))
        cur = u;
      u = _data[u].target;
    }
    return cur;
  }

  size_t reroot(size_t u) {
    auto p = u;
    if (p == u)
      return u;
    auto v = reroot(p);
    _data[v].target = u;
    return u;
  }
};
