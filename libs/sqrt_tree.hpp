#pragma once
#include <cstdint>
#include <set>
#include <vector>
#include <bit>

inline int log2Up(int n) {
  int res = 0;
  while ((1 << res) < n) {
    res++;
  }
  return res;
}

template <typename T>
class sqrt_tree {
private:
  using value_type = T;
  using size_type = int64_t;
  auto op(auto x, auto y) { return x.second>y.second?x:y;}

  size_type n, lg, indexSz;
  std::vector<T> v;
  std::vector<size_type> clz, layers, onLayer;
  std::vector<std::vector<T> > pref, suf, between;

  void buildBlock(size_type layer, size_type l, size_type r) {
    pref[layer][l] = v[l];
    for (size_type i = l + 1; i < r; i++) {
      pref[layer][i] = op(pref[layer][i - 1], v[i]);
    }
    suf[layer][r - 1] = v[r - 1];
    for (size_type i = r - 2; i >= l; i--) {
      suf[layer][i] = op(v[i], suf[layer][i + 1]);
    }
  }

  void buildBetween(size_type layer, size_type lBound, size_type rBound, size_type betweenOffs) {
    size_type bSzLog = (layers[layer] + 1) >> 1;
    size_type bCntLog = layers[layer] >> 1;
    size_type bSz = 1 << bSzLog;
    size_type bCnt = (rBound - lBound + bSz - 1) >> bSzLog;
    for (size_type i = 0; i < bCnt; i++) {
      T ans;
      for (size_type j = i; j < bCnt; j++) {
        T add = suf[layer][lBound + (j << bSzLog)];
        ans = (i == j) ? add : op(ans, add);
        between[layer - 1][betweenOffs + lBound + (i << bCntLog) + j] = ans;
      }
    }
  }

  void buildBetweenZero() {
    size_type bSzLog = (lg + 1) >> 1;
    for (size_type i = 0; i < indexSz; i++) {
      v[n + i] = suf[0][i << bSzLog];
    }
    build(1, n, n + indexSz, (1 << lg) - n);
  }

  void updateBetweenZero(size_type bid) {
    size_type bSzLog = (lg + 1) >> 1;
    v[n + bid] = suf[0][bid << bSzLog];
    update(1, n, n + indexSz, (1 << lg) - n, n + bid);
  }

  void build(size_type layer, size_type lBound, size_type rBound, size_type betweenOffs) {
    if (layer >= (size_type)layers.size()) {
      return;
    }
    size_type bSz = 1 << ((layers[layer] + 1) >> 1);
    for (size_type l = lBound; l < rBound; l += bSz) {
      size_type r = std::min(l + bSz, rBound);
      buildBlock(layer, l, r);
      build(layer + 1, l, r, betweenOffs);
    }
    if (layer == 0) {
      buildBetweenZero();
    } else {
      buildBetween(layer, lBound, rBound, betweenOffs);
    }
  }

  void update(size_type layer, size_type lBound, size_type rBound, size_type betweenOffs, size_type x) {
    if (layer >= (size_type)layers.size()) {
      return;
    }
    size_type bSzLog = (layers[layer] + 1) >> 1;
    size_type bSz = 1 << bSzLog;
    size_type blockIdx = (x - lBound) >> bSzLog;
    size_type l = lBound + (blockIdx << bSzLog);
    size_type r = std::min(l + bSz, rBound);
    buildBlock(layer, l, r);
    if (layer == 0) {
      updateBetweenZero(blockIdx);
    } else {
      buildBetween(layer, lBound, rBound, betweenOffs);
    }
    update(layer + 1, l, r, betweenOffs, x);
  }

  T query(size_type l, size_type r, size_type betweenOffs, size_type base) {
    if (l == r) {
      return v[l];
    }
    if (l + 1 == r) {
      return op(v[l], v[r]);
    }
    size_type layer = onLayer[clz[(l - base) ^ (r - base)]];
    size_type bSzLog = (layers[layer] + 1) >> 1;
    size_type bCntLog = layers[layer] >> 1;
    size_type lBound = (((l - base) >> layers[layer]) << layers[layer]) + base;
    size_type lBlock = ((l - lBound) >> bSzLog) + 1;
    size_type rBlock = ((r - lBound) >> bSzLog) - 1;
    T ans = suf[layer][l];
    if (lBlock <= rBlock) {
      T add =
          (layer == 0) ? (query(n + lBlock, n + rBlock, (1 << lg) - n, n))
                       : (between[layer - 1][betweenOffs + lBound +
                                             (lBlock << bCntLog) + rBlock]);
      ans = op(ans, add);
    }
    ans = op(ans, pref[layer][r]);
    return ans;
  }

 public:
  T query(size_type l, size_type r) { return query(l, r, 0, 0); }

  void update(size_type x, const T &item) {
    v[x] = item;
    update(0, 0, n, 0, x);
    std::set<size_type> a;
  }

  sqrt_tree()=default;
  sqrt_tree(const std::vector<T> &a)
      : n((size_type)a.size()), lg(log2Up(static_cast<size_t>(n))), v(a), clz(1 << lg), onLayer(lg + 1) {
    clz[0] = 0;
    for (size_type i = 1; i < (size_type)clz.size(); i++) {
      clz[i] = clz[i >> 1] + 1;
    }
    size_type tlg = lg;
    while (tlg > 1) {
      onLayer[tlg] = (size_type)layers.size();
      layers.push_back(tlg);
      tlg = (tlg + 1) >> 1;
    }
    for (size_type i = lg - 1; i >= 0; i--) {
      onLayer[i] = std::max(onLayer[i], onLayer[i + 1]);
    }
    size_type betweenLayers = std::max((size_type)0,(size_type)(layers.size() - 1));
    size_type bSzLog = (lg + 1) >> 1;
    size_type bSz = 1 << bSzLog;
    indexSz = (n + bSz - 1) >> bSzLog;
    v.resize(n + indexSz);
    pref.assign(layers.size(), std::vector<T>(n + indexSz));
    suf.assign(layers.size(), std::vector<T>(n + indexSz));
    between.assign(betweenLayers, std::vector<T>((1 << lg) + bSz));
    build(0, 0, n, 0);
  }
};
