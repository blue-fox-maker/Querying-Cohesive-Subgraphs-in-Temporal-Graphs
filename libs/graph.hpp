#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <map>
#include <ranges>
#include <set>
#include <vector>
#include <type_traits>
#include <numeric>

// decay ?

// candy structures
template <typename T>
struct attributed{
  T attr;
  attributed()=default;
};
template <>
struct attributed<void>{};

template <typename T = void>
struct vert:attributed<T> {
  size_t id;
  vert()=default;
  template <typename U>
  auto operator<=>(const vert<U> &other) const {
    return id<=>other.id;
  }
};
template <typename T = void>
struct target_edge:attributed<T> {
  size_t target;
  target_edge()=default;
  template <typename U>
  auto operator<=>(const target_edge<U>& other) const {
    return target<=>other.target;
  }
};
template <typename T = void>
struct edge:attributed<T> {
  size_t source;
  size_t target;
  edge()=default;
  template <typename U>
  auto operator<=>(const edge<U>& other) const {
    return std::pair{source,target}<=>std::pair{other.source,other.target};
  }
};

// candy functions
template <typename VA = void>
auto make_vert(size_t v, auto&&...attr_args){
  auto res = vert<VA>{};
  res.id = v;
  if constexpr (sizeof...(attr_args) != 0)
    res.attr = VA{std::forward<decltype(attr_args)>(attr_args)...};
  return res;
}
template <typename EA = void>
auto make_target_edge(size_t v, auto&&...attr_args){
  auto res = target_edge<EA>{};
  res.target = v;
  if constexpr (sizeof...(attr_args) != 0)
    res.attr = EA{std::forward<decltype(attr_args)>(attr_args)...};
  return res;
}
template <typename EA = void>
auto make_edge(size_t u,size_t v, auto&&...attr_args){
  auto res = edge<EA>{};
  res.source = u;
  res.target = v;
  if constexpr (sizeof...(attr_args) != 0)
    res.attr = EA{std::forward<decltype(attr_args)>(attr_args)...};
  return res;
}

// graph concept
template <typename T>
concept VertListGraph = requires (T t,size_t v){
  {vert_capacity(t)}->std::convertible_to<size_t>;
  {verts(t)}->std::ranges::view;  
  {vattr(t,v)}->std::same_as<std::remove_reference_t<decltype(vattr(t, v))>&>;
  {target_edges(t,v)}->std::ranges::view;
};

template <typename T>
concept EdgeListGraph = false;

// traits
template <typename T>
struct graph_traits{
  using vattr_t = typename T::vattr_t;
  using eattr_t = typename T::eattr_t;
};

// basic access function
auto vert_capacity(const auto& g) {return g.vert_capacity();}
auto verts(const auto &g) { return g.verts(); }
auto target_edges(const auto &g, size_t v) { return g.target_edges(v);}
decltype(auto) vattr(const auto &g, size_t v) { return g.vattr(v);}

// more access function
size_t num_vert(const auto &g) {return std::ranges::size(verts(g));}
auto degree(const auto &g, size_t v)-> size_t { return std::ranges::distance(target_edges(g,v));}
auto target_verts(const auto &g, size_t v) { return target_edges(g,v)|std::views::transform([](const auto&e){return e.target;});}


// graph container
template <typename VA, typename EA>
class adjacent_graph {
public:
  using vattr_t = VA;
  using eattr_t = EA;
private:
  struct node:attributed<VA> {
    std::vector<target_edge<EA>> target_edges;
  };
  std::vector<node> _data;
public:
  adjacent_graph()=default;
  adjacent_graph(VertListGraph auto &&g){
    for(size_t v: verts(g)){
      if(_data.size()<=v) _data.resize(v+1);
      _data[v].target_edges = {std::ranges::begin(target_edges(g,v)),std::ranges::end(target_edges(g,v))};
      if constexpr (!std::is_void_v<VA>)
        _data[v].attr = vattr(g,v);
    }
  }
  auto vert_capacity() const -> size_t { return _data.size();}
  auto verts() const { return std::views::iota(0U,_data.size());}
  auto& vattr(size_t v) const { return _data[v].attr;}
  auto target_edges(size_t v) const { return std::ranges::subrange{_data[v].target_edges.begin(),_data[v].target_edges.end()}; }
  void add_vert(size_t u, auto &&...attr_args){
    if (_data.size() <= u) 
      _data.resize(u+1);
    if constexpr (sizeof...(attr_args)!=0)
      _data[u].attr = VA{std::forward<decltype(attr_args)>(attr_args)...};
  }
  void add_edge(size_t u, size_t v, auto &&...attr_args){
    _data[u].target_edges.push_back(make_target_edge<EA>(v,std::forward<decltype(attr_args)>(attr_args)...));
  }
  void remove_edge(size_t u, size_t v){
    std::erase_if(_data[u].target_edges,[v](const target_edge<EA> &e){ return e.target == v;});
  }
};
template <VertListGraph T>
adjacent_graph(T&&)->adjacent_graph<typename graph_traits<T>::vattr_t,typename graph_traits<T>::eattr_t>;

template <typename VA, typename EA> class dynamic_graph {
  std::map<vert<VA>, std::set<target_edge<EA>>> _data;
public:
  using vattr_t =VA;
  using eattr_t = EA;
  dynamic_graph()=default;
  dynamic_graph(VertListGraph auto && g){
    for(size_t u: ::verts(g)){
      if constexpr (!std::is_void_v<VA>)
        _data[make_vert<VA>(u, ::vattr(g,u))] = ::target_edges(g,u)|std::ranges::to<std::set<target_edge<EA>>>();
      else 
        _data[make_vert(u)] = ::target_edges(g,u)|std::ranges::to<std::set<target_edge<EA>>>();
    }
  }
  auto verts() const { return _data|std::views::keys|std::views::transform(&vert<VA>::id); }
  auto vert_capacity() const -> size_t { return _data.empty()?0:_data.rbegin()->first.id+1;}
  auto& vattr(size_t u) const { return _data.find(make_vert<VA>(u))->first.attr;}
  auto target_edges(size_t u) const { auto & temp = _data.at(make_vert<VA>(u)); return std::ranges::subrange(temp.begin(),temp.end()); }
  void add_vert(size_t u, auto &&... attr_args) {
    _data[make_vert<VA>(u,std::forward<decltype(attr_args)>(attr_args)...)] = {};
  }
  void add_edge(size_t u, size_t v, auto &&...attr_args) {
    _data[u].insert(make_target_edge(v,std::forward<decltype(attr_args)>(attr_args)...));
  }
  void remove_vert(size_t u) {
    if(!_data.contains(make_vert<VA>(u))) return;
    for(auto &&e:_data[make_vert<VA>(u)])
      remove_edge(e.target,u);
    _data.erase(make_vert<VA>(u));
  }
  void remove_vert(const std::ranges::range auto& rng){
    for(size_t u:rng) remove_vert(u);
  }
  void remove_edge(size_t u, size_t v) {
    if(_data.contains(make_vert<VA>(u)))
      _data[make_vert<VA>(u)].erase(make_target_edge<EA>(v));
  }
};
template <VertListGraph T>
dynamic_graph(T&&)->dynamic_graph<typename graph_traits<T>::vattr_t,typename graph_traits<T>::eattr_t>;

// graph maintainer
template <typename VA, typename EA> class spanning_forest {
  using vattr_t = VA;
  using eattr_t = EA;
  struct node : public attributed<VA> {
    target_edge<EA> path;
    size_t rank;
  };
  std::vector<node> _data;

  size_t find(size_t u) {
    return u == _data[u].path.target ? u : find(_data[u].path.target);
  }

public:

  spanning_forest(auto &&g){
    for(size_t v: verts(g)){
      if(_data.size()<=v) _data.resize(v+1);
      if constexpr (!std::is_void_v<VA>)
        _data[v].attr = vattr(g,v);
    }
    for(size_t v: verts(g)){
      for(auto &&tar_e: target_edges(g,v)){
        if constexpr (std::is_void_v<EA>)
          add_edge(v,tar_e.target);
        else
          add_edge(v,tar_e.target,tar_e.attr);
      }
    }
  }

  auto vert_capacity() const -> size_t { return _data.size();}
  auto verts() const { return std::views::iota(0,_data.size());}
  auto vattr(size_t v) const -> VA& { return _data[v].attr;}
  auto target_edges(size_t v) const { return std::views::single(_data[v].path);}

  void add_edge(size_t u, size_t v, auto &&...attr_args){
    u = find(u);
    v = find(v);
    if(u==v) return;
    if (_data[u].rank > _data[v].rank)
      std::swap(u, v);
    _data[u].path = make_target_edge<EA>(v,std::forward<decltype(attr_args)>(attr_args)...);
    _data[v].rank += _data[u].rank;
  }
};

template <typename VA, typename EA, typename WProj = std::identity> requires (!std::is_void_v<EA>)
class minimum_spanning_forest {

  using vattr_t = VA;
  using eattr_t = EA;
private:
  struct node : public attributed<VA> {
    target_edge<EA> path;
    size_t depth;
  };
  std::vector<node> _data;

public:
  minimum_spanning_forest(auto &&g){
    for(size_t v: verts(g)){
      if(_data.size()<=v) _data.resize(v+1);
      if constexpr (!std::is_void_v<VA>)
        _data[v].attr = vattr(g,v);
    }
    for(size_t v: verts(g)){
      for(auto &&tar_e: target_edges(g,v)){
          add_edge(v,tar_e.target,tar_e.attr);
      }
    }
  }
  auto vert_capacity() const -> size_t { return _data.size();}
  auto verts() const { return std::views::iota(0,_data.size());}
  auto& vattr(size_t v) const { return _data[v].attr;}
  auto target_edges(const size_t v) const { return std::views::single(v);}
  void add_edge(size_t u, size_t v, auto &&...attr_args) {
    if (u==v) return ;
    auto w = critical_edge(u,v);
    if (_data[u].depth > _data[v].depth) std::swap(u,v);
    if (w){
      if (WProj{}(_data[*w].path.attr) < WProj{}(_data[*w].path.attr)) return;
      _data[*w].path.target = *w;
    }
    reroot(u);
    _data[u].path.target = v;
    _data[u].path.attr = EA{std::forward<decltype(attr_args)>(attr_args)...};
    return w;
  }

private:
  size_t find(size_t v) {
    if (auto p = _data[v].path.target; p != v) {
      auto res = find(p);
      _data[v].depth = _data[p].depth + 1;
      return res;
    }
    _data[v].depth = 0;
    return v;
  }

  std::optional<size_t> critical_edge(size_t u, size_t v) {
    if (find(u) != find(v))
      return {};
    std::optional<size_t> cur;
    while (u != v) {
      if (_data[u].depth < _data[v].depth)
        std::swap(u,v);
      if (!cur || WProj{}(_data[*cur].path.attr) < WProj{}(_data[*cur].path.attr))
        cur = u;
      u = _data[u].path.target; // TODO correct?
    }
    return cur;
  }

  size_t reroot(size_t u) {
    auto p = u;
    if (p == u)
      return u;
    auto v = reroot(p);
    _data[v].path.target = u;
    vattr(v) = vattr(u);
    return u;
  }
};

// lazy views

namespace views {

template <typename G, typename Pred> requires(!std::is_void_v<typename graph_traits<G>::eattr_t>)
struct edge_induce_view {
  using vattr_t = graph_traits<G>::vattr_t;
  using eattr_t = graph_traits<G>::eattr_t;
  const G& base;
  Pred pred;
  auto vert_capacity() const -> size_t { return ::vert_capacity(base); }
  auto verts() const { return ::verts(base)|std::views::filter([this](const size_t v){ return degree(base,v)!=0; });}
  auto& vattr(size_t v) const { return ::vattr(base,v);}
  auto target_edges(size_t v) const { return ::target_edges(base,v)|std::views::filter([this](const auto& e){ return pred(e.attr);}); }
};
template <typename G> 
struct trivial_view {
  using vattr_t = graph_traits<G>::vattr_t;
  using eattr_t = void;
  const G& base;
  auto vert_capacity() const -> size_t { return ::vert_capacity(base); }
  auto verts() const { return ::verts(base); }
  auto &vattr(size_t v) const { return ::vattr(base,v); }
  auto target_edges(size_t v) const { return ::target_edges(base,v)|std::views::transform([this](const auto& e){ return make_target_edge(e.target);});}
};
template <typename G>
struct refined_view {
  using vattr_t = graph_traits<G>::vattr_t;
  using eattr_t = graph_traits<G>::eattr_t;
  const G& base;
  std::vector<size_t> new_to_old;
  std::vector<size_t> old_to_new;
  refined_view(const G& g):base(g),old_to_new(::vert_capacity(g)), new_to_old(num_vert(g)){
    size_t i = 0;
    for(size_t v: ::verts(g)){
      new_to_old[i]=v;
      old_to_new[v]=i;
      i++;
    }
  }
  refined_view(const refined_view&)=delete;
  auto vert_capacity() const -> size_t { return new_to_old.size(); }
  auto verts() const { return std::views::iota(0U,new_to_old.size()); }
  auto&vattr(size_t v) const { return ::vattr(base,new_to_old[v]);}
  auto target_edges(size_t v) const { 
    return ::target_edges(base, new_to_old[v])|std::views::transform([this](const target_edge<eattr_t>&e){ 
      if constexpr (!std::is_void_v<eattr_t>)
         return make_target_edge<eattr_t>(old_to_new[e.target],e.attr);  
      else return make_target_edge<eattr_t>(old_to_new[e.target]); });   
  }
};

auto edge_induce(const VertListGraph auto &g, auto &&pred){
  return edge_induce_view{g,std::forward<decltype(pred)>(pred)};
}
auto trivial(const VertListGraph auto& g){
  return trivial_view{g};
}
auto refined(const VertListGraph auto& g){
  return refined_view{g};
}

}

// algorithm

auto core_decomposition(const auto &g, size_t k = std::numeric_limits<size_t>::max()) -> std::vector<size_t> {
  assert(vert_capacity(g)==num_vert(g));
  if(num_vert(g)==0) return {};
  size_t n = vert_capacity(g);
  auto deg = std::vector<size_t>(n);
  auto vtx = std::vector<size_t>(n);
  auto pos = std::vector<size_t>(n);
  for (auto &&v : verts(g)) {
    deg[v] = degree(g, v);
  }
  size_t max_deg = std::ranges::max(deg);
  auto bin = std::vector<size_t>(max_deg + 2);
  for (auto &&d : deg) {
    bin[d + 1]++;
  }
  bin.back() = 0;
  std::partial_sum(bin.begin(), bin.end(), bin.begin());
  for (auto &&v : verts(g)) {
    pos[v] = bin[deg[v]];
    vtx[pos[v]] = v;
    bin[deg[v]]++;
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

auto k_core(const auto &g, size_t k) -> std::vector<size_t> {
  auto core_numbers = core_decomposition(g, k);
  auto temp = verts(g) | std::views::filter([&](const auto v) { return core_numbers[v] >= k; });
  std::vector<size_t> res;
  std::ranges::move(temp,std::back_inserter(res));
  return res;
}

template <typename Proj = std::identity>
auto temporal_k_core_te_anchored(VertListGraph auto &&g, size_t k, size_t ts, size_t te, const auto& temporal_edges, auto lifetime, Proj&& proj = {}) {
  auto instance = std::forward<decltype(g)>(g);
  for(size_t t = ts; t<te; t++){
    auto newbee = std::vector<size_t>{};
    auto core = k_core(views::refined(instance),k);
    auto cur_verts = verts(instance)|std::ranges::to<std::vector<size_t>>();
    for(size_t i = 0; i < core.size(); i++){
      core[i]=cur_verts[i];
    }
    std::ranges::set_difference(verts(instance),core,std::back_inserter(newbee));
    instance.remove_vert(newbee);
    if (std::ranges::empty(verts(instance))) break;
    for(auto &&e: temporal_edges[t-ts]){
      if((--lifetime[e]) == 0){
        instance.remove_edge(e.source,e.target);
      }
    }
  }
  return 0;
}

template <typename Proj = std::identity>
auto temporal_k_core_update(VertListGraph auto &&g, size_t k, size_t ts, size_t te, size_t tmax, Proj proj = {}){
  auto temporal_edges = std::vector<std::vector<edge<>>>(tmax-ts);
  auto lifetime = std::map<edge<>,size_t>{};
  for(auto v: verts(g)){
    for(auto e: target_edges(g,v)){
      if(auto t = proj(e.attr); ts<=t && t< tmax){
        auto cur_edge = make_edge(v,e.target);
        temporal_edges[t].push_back(cur_edge);
        lifetime[cur_edge]++;
      }
    }
  }
  auto instance = dynamic_graph{views::trivial(views::edge_induce(g, [=,&proj](const auto &attr){ auto t = proj(attr); return ts<=t && t<=tmax; }))};
  auto temporal_edges_view = std::span{temporal_edges};
  for(size_t cur_te : std::views::iota(te,tmax)|std::views::reverse){
    auto core = k_core(views::refined(instance),k);
    auto cur_verts = verts(instance)|std::ranges::to<std::vector<size_t>>();
    auto newbee = std::vector<size_t>{};
    for(size_t i = 0; i < core.size(); i++){ core[i]=cur_verts[i]; }
    std::ranges::set_difference(verts(instance),core,std::back_inserter(newbee));
    instance.remove_vert(newbee);
    if(std::ranges::empty(verts(instance))) break;
    for(auto &&e: temporal_edges[cur_te])
      if((--lifetime[e]) == 0)
        instance.remove_edge(e.source,e.target);
    temporal_k_core_te_anchored(instance,k,ts,cur_te,temporal_edges,lifetime,proj);
  }
  return 0;
}
