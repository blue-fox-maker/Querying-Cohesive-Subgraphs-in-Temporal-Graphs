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

// TODO add const
// make directed graph a view

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

// template <typename T>
// struct graph_interface{
//   auto verts() { return reinterpret_cast<T*>(this)->verts(); }
//   auto edges() { return reinterpret_cast<T*>(this)->edges(); }

//   auto source_edges(size_t u) { return reinterpret_cast<T*>(this)->source_edges(u); }
//   auto target_edges(size_t u) { return reinterpret_cast<T*>(this)->target_edges(u); }

//   void insert_vert(size_t u) { return reinterpret_cast<T*>(this)->insert_vert(u); }
//   void remove_vert(size_t u) { return reinterpret_cast<T*>(this)->remove_vert(u); }
//   void insert_edge(size_t u, size_t v, auto ea) { return reinterpret_cast<T*>(this)->insert_edge(u,v,std::move(ea)); }
//   void remove_edge(size_t u, size_t v) { return reinterpret_cast<T*>(this)->remove_edge(u,v); }

//   auto incident_edges(size_t u) { if constexpr(requires{target_edges(u);}) return target_edges(u); else return source_edges(u); }

//   auto num_vert()->size_t { return std::ranges::distance(verts()); }
//   auto num_edge()->size_t { return std::ranges::distance(edges()); }
//   auto source_degree(size_t u)->size_t { return std::ranges::distance(source_edges(u)); }
//   auto target_degree(size_t u)->size_t { return std::ranges::distance(target_edges(u)); }
//   auto degree(size_t u)->size_t { return std::ranges::distance(incident_edges(u));}
//   auto max_source_degree()->size_t { return std::ranges::max(verts()|std::views::transform([this](size_t u){return source_degree(u);}));}
//   auto max_target_degree()->size_t { return std::ranges::max(verts()|std::views::transform([this](size_t u){return target_degree(u);}));}
//   auto max_degree()->size_t { return std::ranges::max(verts()|std::views::transform([this](size_t u){return degree(u);}));}
//   auto target_verts(size_t u) { return target_edges(u)|std::views::transform([](const auto& x){return x.target;}); }
//   auto source_verts(size_t u) { return source_edges(u)|std::views::transform([](const auto& x){return x.source;}); }
//   auto neighbors(size_t u) { if constexpr(requires{target_edges(u);}) return target_verts(u); else return source_verts(u);}

//   private:
//   auto self() { return reinterpret_cast<T*>(this);}
// };

// template <typename EA = none>
// class edgelist_graph: public graph_interface<edgelist_graph<EA>> {
// public:
//   using eattr = EA;
//   auto edges() const { return std::ranges::subrange{_data.begin(), _data.end()}; }
//   void insert_edge(size_t u, size_t v, auto ea) { _data.emplace_back(u,v,std::move(ea)); }
//   edgelist_graph()=default;
//   edgelist_graph(auto &&g){ for(auto e: g.edges()) insert_edge(e.source,e.target,std::move(e.attr)); }
// private:
//   std::vector<edge<EA>> _data;
// };

// template <typename EA = none> // undirected
// class adjacent_graph: public graph_interface<adjacent_graph<EA>> {
// public:
//   // using eattr = EA;
//   auto& eattr(size_t u, size_t v) { return _data[u][v];}
//   auto verts() { return std::views::iota(0U, _data.size()); }
//   auto target_edges(size_t u) { return _data[u]|std::views::transform([](auto&x){ return target_edge<EA>{x.first,x.second}; }); }
//   void insert_vert(size_t u){ if (u>=_data.size()) _data.resize(u+1); }
//   void insert_edge(size_t u, size_t v, auto ea){ insert_vert(u); insert_vert(v); _data[u][v]=ea; _data[v][u]=std::move(ea); }
//   void remove_edge(size_t u, size_t v) { _data[v].erase(u);_data[u].erase(v);}
//   void remove_vert(size_t u) { for(auto v: this->neighbors(u)) _data[v].erase(u); _data[u].clear(); }
//   adjacent_graph()=default;
//   adjacent_graph(auto &&g){ for(auto e: g.edges()) insert_edge(e.source,e.target,std::move(e.attr)); }
// private:
//   std::vector<std::map<size_t,EA>> _data;
//   // std::vector<std::set<target_edge<EA>>> _data;
// };

// template <typename EA> 
// class spanning_forest{
//   std::vector<size_t> _rank;
//   std::vector<target_edge<EA>> _data;

// public:
//   size_t num_vert() const noexcept { return _data.size(); }
//   template <std::ranges::range R = std::ranges::empty_view<edge<EA>>>
//   spanning_forest(size_t num_vert, R&&rng = {}):_data(num_vert),_rank(num_vert){
//     for(size_t i = 0; i < _data.size(); i++) _data[i] = target_edge<EA>{i};
//     for(auto&& e:rng) add_edge(e.source,e.target,e.attr);
//   }
//   void add_edge(size_t u, size_t v, auto &&...attr_args){
//     u = find(u);
//     v = find(v);
//     if(u==v) return;
//     if (_rank[u]> _rank[v])
//       std::swap(u, v);
//     _data[u] = target_edge<EA>{v,std::forward<decltype(attr_args)>(attr_args)...};
//     _rank[v]+= _rank[u];
//   }
// private:
//   size_t find(size_t u) {
//     return u == _data[u].target ? u : find(_data[u].target);
//   }
// };

template <typename EA, typename WProj = std::identity> 
class minimum_spanning_forest {
  std::vector<target_edge<EA>> _data;
  std::vector<size_t> _depth; // this array is useless. I need to get rid of it
public:
  size_t num_vert() const noexcept { return _data.size(); }
  auto edges() const noexcept { return std::views::iota(0U,num_vert())|std::views::transform([this](size_t i){return edge{i,_data[i].target,std::ref(_data[i].attr)};});}
  auto edges() noexcept { return std::views::iota(0U,num_vert())|std::views::transform([this](size_t i){return edge{i,_data[i].target,std::ref(_data[i].attr)};});}
  template <std::ranges::range R = std::ranges::empty_view<edge<EA>>>
  minimum_spanning_forest(size_t num_vert, R&& rng = {}):_data(num_vert),_depth(num_vert,0){
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

// // adaptor
// template <typename G>
// class k_core: graph_interface<k_core<G>> {
//   size_t k;
//   G& g;
// public:
//   auto verts() { return g.verts()| std::views::filter([this](size_t u){ return g.degree(u)>=k;}); }
//   auto edges() { return g.edges(); }
//   auto& eattr(size_t u, size_t v) { return g.eattr(u,v); }
//   auto target_edges(size_t u) { return g.target_edges(u); }
//   k_core(G& g, size_t k):g(g),k(k){
//     std::vector<size_t> cur;
//     for(auto v: g.verts()) if (g.degree(v)<k) cur.push_back(v);
//     remove_verts(cur);
//   }
//   auto remove_edge(size_t u, size_t v){
//     g.remove_edge(u,v);
//     std::vector<size_t> cur;
//     if(g.degree(u)==k-1) cur.push_back(u);
//     if(g.degree(v)==k-1) cur.push_back(v);
//     remove_verts(cur);
//   }
//   auto remove_verts(auto&& rng){
//     std::queue<size_t> cur;
//     for(size_t v:rng) cur.push(v);
//     while(!cur.empty()){
//       auto u = cur.front();
//       cur.pop();
//       for(auto v: g.neighbors(u))
//         if(g.degree(v)==k) cur.push(v);
//       g.remove_vert(u);
//     }
//   }
// };

