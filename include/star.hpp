#include <algorithm>
#include <compare>
#include <iterator>
#include <ranges>
#include <vector>
#include <numeric>
#include "concat.hpp"

static constexpr size_t invalid_index = -1;

struct none {
  friend auto operator<=>(none,none)=default;
};

template <typename T = none> struct target_edge {
  using attr_t = T;
  size_t target;
  [[no_unique_address]] T attr;
  target_edge(size_t v, auto &&...args):target(v),attr(std::forward<decltype(args)>(args)...){}
  target_edge()=default;
};
template <typename T = none> struct source_edge {
  using attr_t = T;
  size_t source;
  [[no_unique_address]] T attr;
  source_edge(size_t v, auto &&...args):source(v),attr(std::forward<decltype(args)>(args)...){}
  source_edge()=default;
};
template <typename T = none> struct edge {
  using attr_t = T;
  size_t source;
  size_t target;
  [[no_unique_address]] T attr;
  edge(size_t u, size_t v, auto &&...args):source(u),target(v),attr(std::forward<decltype(args)>(args)...){}
  edge(size_t u, const source_edge<T>& x):source(x.source),target(u),attr(x.attr){}
  edge(size_t u, const target_edge<T>& x):source(u),target(x.target),attr(x.attr){}
  edge()=default;
};
template <typename T>
edge(size_t, size_t, T&&)->edge<T>;


template <typename T>
class graph_interface {
  public:
    auto neighbors(size_t i) { return std::views::concat(static_cast<T*>(this)->source_neighbors(i), static_cast<T*>(this)->target_neighbors(i));}
    size_t degree(size_t i) { 
      return std::ranges::size(static_cast<T*>(this)->source_degree(i))+std::ranges::size(static_cast<T*>(this)->target_degree(i));
    }
    auto verts() { return std::views::iota(static_cast<T*>(this)->num_vert()); }
  protected: graph_interface() = default;
};

template <std::ranges::random_access_range U>
struct index_iterator {
  using value_type = size_t;
  size_t cur;  
  U& next;
  explicit index_iterator(const U& next, size_t index):cur(index),next(next){}
  size_t operator*(){ return cur; }
  index_iterator& operator++() { cur = next[cur]; }
  index_iterator operator++(int) { auto temp = *this; cur = next[cur]; return temp; }
  bool operator==(std::default_sentinel_t){ return cur == invalid_index;}
};

template <typename EA> class graph:public graph_interface<graph<EA>> {
public:
  static constexpr bool Source = true;
  static constexpr bool Target = true;
  static constexpr bool Degree = true;
  using edge_t = edge<EA>;
  using vert_t = size_t;

private:
  size_t _num_vert;
  std::vector<edge_t> _data;
  [[no_unique_address]] std::conditional_t<Source, std::vector<size_t>, none> _source_entry;
  [[no_unique_address]] std::conditional_t<Source, std::vector<size_t>, none> _source_prev;
  [[no_unique_address]] std::conditional_t<Source, std::vector<size_t>, none> _source_next;
  [[no_unique_address]] std::conditional_t<Target, std::vector<size_t>, none> _target_entry;
  [[no_unique_address]] std::conditional_t<Source, std::vector<size_t>, none> _target_prev;
  [[no_unique_address]] std::conditional_t<Target, std::vector<size_t>, none> _target_next;
  [[no_unique_address]] std::conditional_t<Source && Degree, std::vector<size_t>, none> _source_degree;
  [[no_unique_address]] std::conditional_t<Target && Degree, std::vector<size_t>, none> _target_degree;

  void init_entry_list(auto &entry, auto &next, auto &&proj) {
    for (auto &&[i, e] : _data | std::views::enumerate | std::views::reverse) {
      auto val = func(e);
      if (entry[val] != invalid_index)
        next[entry[val]] = i;
      entry[val] = i;
    }
  }
  void remove_entry_list(size_t &entry, auto& prev, auto& next, size_t i){
    if(entry==i) entry = next[i];
    prev[next[i]] = prev[i];
    next[prev[i]] = next[i];
  }

public:
  size_t num_vert() const noexcept { return _num_vert; }
  size_t source_degree(size_t i) const noexcept { return _source_degree[i]; }
  size_t target_degree(size_t i) const noexcept { return _target_degree[i]; }
  template <std::ranges::range R = std::ranges::empty_view<edge<EA>>>
  graph(size_t num_vert, R &&rng = {})
      : _num_vert(num_vert),
        _data(std::ranges::begin(rng), std::ranges::end(rng)),
        _source_entry(num_vert,invalid_index), _source_prev(std::ranges::size(rng),invalid_index),_source_next(std::ranges::size(rng),invalid_index),
        _target_entry(num_vert,invalid_index), _target_prev(std::ranges::size(rng),invalid_index),_target_next(std::ranges::size(rng),invalid_index),
        _source_degree(num_vert), _target_degree(num_vert) {
    if constexpr (Source) init_entry_list(_source_entry,_source_next,&edge_t::source);
    if constexpr (Target) init_entry_list(_target_entry,_target_next,&edge_t::target);
    if constexpr (Source && Degree) for (size_t i = 0; i < num_vert; i++) _source_degree[i] = std::ranges::distance(std::ranges::subrange(index_iterator{_data,_source_next,_source_entry[i]},std::default_sentinel));
    if constexpr (Target && Degree) for (size_t i = 0; i < num_vert; i++) _target_degree[i] = std::ranges::distance(std::ranges::subrange(index_iterator{_data,_target_next,_target_entry[i]},std::default_sentinel));
  }
  void remove_edge(size_t i){
    if constexpr (Source) remove_entry_list(_source_entry[_data[i].source],_source_prev,_source_next,i);
    if constexpr (Target) remove_entry_list(_target_entry[_data[i].target],_target_prev,_target_next,i);
    if constexpr (Source && Degree) _source_degree[_data[i].source]--;
    if constexpr (Target && Degree) _target_degree[_data[i].target]--;
  }
  void remove_vert(size_t i){
    if constexpr (Source) for (size_t j = _source_entry[i]; j!=invalid_index; j=_source_next[j]) remove_edge(j);
    if constexpr (Target) for (size_t j = _target_entry[i]; j!=invalid_index; j=_target_next[j]) remove_edge(j);
  }
};

template <std::ranges::range R>
graph(size_t , R &&)->graph<typename std::ranges::range_value_t<R>::attr_t>;


template <typename EA> 
class spanning_forest: public graph_interface<spanning_forest<EA>> {
  std::vector<size_t> _rank;
  std::vector<target_edge<EA>> _data;

public:
  size_t num_vert() const noexcept { return _data.size(); }
  template <std::ranges::range R = std::ranges::empty_view<edge<EA>>>
  spanning_forest(size_t num_vert, R&&rng = {}):_data(num_vert),_rank(num_vert){
    for(size_t i = 0; i < _data.size(); i++) _data[i] = target_edge<EA>{i};
    for(auto&& e:rng) add_edge(e.source,e.target,e.attr);
  }
  void add_edge(size_t u, size_t v, auto &&...attr_args){
    u = find(u);
    v = find(v);
    if(u==v) return;
    if (_rank[u]> _rank[v])
      std::swap(u, v);
    _data[u] = target_edge<EA>{v,std::forward<decltype(attr_args)>(attr_args)...};
    _rank[v]+= _rank[u];
  }
private:
  size_t find(size_t u) {
    return u == _data[u].target ? u : find(_data[u].target);
  }
};

template <typename EA, typename WProj = std::identity> 
class minimum_spanning_forest: public graph_interface<minimum_spanning_forest<EA,WProj>>{
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
    if (_depth[_data[u]] >_depth[_data[v]]) std::swap(u,v);
    if (w){
      if (WProj{}(_data[*w].attr) < WProj{}(_data[*w].attr)) return std::nullopt;
      _data[*w].target = *w;
    }
    reroot(u);
    _data[u].target = v;
    _data[u].attr = target_edge<EA>{std::forward<decltype(attr_args)>(attr_args)...};
    return w;
  }

private:
  size_t find(size_t v) {
    if (auto p = _data[v].target; p != v) {
      auto res = find(p);
      _depth[_data[v]] = _depth[_data[p]] + 1;
      return res;
    }
    _depth[_data[v]] = 0;
    return v;
  }

  std::optional<size_t> critical_edge(size_t u, size_t v) {
    if (find(u) != find(v))
      return {};
    std::optional<size_t> cur;
    while (u != v) {
      if (_depth[_data[u]] < _depth[_data[v]])
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

// algorithm
auto core_decomposition(const auto &g, size_t k = std::numeric_limits<size_t>::max()) -> std::vector<size_t> {
  if(g.num_vert()==0) return {};
  size_t n = g.num_vert();
  auto deg = std::vector<size_t>(n);
  auto vtx = std::vector<size_t>(n);
  auto pos = std::vector<size_t>(n);
  for(size_t i: g.verts()) deg[i] = g.degree(i);
  size_t max_deg = std::ranges::max(deg);
  auto bin = std::vector<size_t>(max_deg + 2);
  for (auto &&d : deg) {
    bin[d + 1]++;
  }
  bin.back() = 0;
  std::partial_sum(bin.begin(), bin.end(), bin.begin());
  for (size_t i:g.verts()) {
    pos[i] = bin[deg[i]];
    vtx[pos[i]] = i;
    bin[deg[i]]++;
  }
  std::rotate(bin.rbegin(), bin.rbegin() + 1, bin.rend());
  for (size_t i = 0; i < n; i++) {
    auto v = vtx[i];
    if (deg[v] >= k)
      break;
    for (auto &&u : target_verts(g, v)) {
      if (deg[u] > deg[v]) {
        auto du = deg[u];
        auto pu = pos[u];
        auto pw = bin[du];
        auto w = vtx[pw];
        if (u != w) {
          pos[u] = pw;
          vtx[pu] = w;
          pos[w] = pu;
          vtx[pw] = u;
        }
        bin[du]++;
        deg[u]--;
      }
    }
  }
  deg.pop_back();
  return deg;
}
