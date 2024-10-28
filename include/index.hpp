#include <vector>
#include <ranges>
#include <span>

template <size_t N = std::dynamic_extent>
struct category_list {
  static constexpr size_t invalid_index = -1;
  struct navi { size_t prev = invalid_index; size_t next = invalid_index; };
  using entry_type = std::conditional_t<N==std::dynamic_extent,std::vector<size_t>,std::array<size_t,N>>;
  struct iterator {
    std::vector<navi>::iterator _base;
    using difference_type = ptrdiff_t;
    size_t cur;
    constexpr size_t operator*(){ return cur; }
    constexpr iterator& operator++(){ cur = (_base+cur)->next; return *this; }
    constexpr iterator& operator--(){ cur = (_base+cur)->prev; return *this; }
    constexpr iterator operator++(int){ auto temp = *this; cur = (_base+cur)->next; return temp; }
    constexpr iterator operator--(int){ auto temp = *this; cur = (_base+cur)->prev; return temp; }
    friend bool operator==(iterator x,std::default_sentinel_t){ return x.cur == invalid_index;}
  };
  [[nodiscard]] constexpr size_t size() const { return _data.size();}
  void insert(size_t index, size_t category = 0){
    if(_data.size()<index+1) _data.resize(index+1);
    auto& entry = _entry[category];
    _data[entry].prev = index;
    _data[index].next = entry;
    entry = index;
  }
  void remove(size_t index,size_t category = 0){
    if(_entry[category] == index) _entry[category] = _data[index].next;
    else _data[_data[index].prev].next = _data[index].next;
    _data[_data[index].next].prev = _data[index].prev;
  }
  auto category(size_t x = 0){ return std::ranges::subrange{iterator{_data.begin(),_entry[x]},std::default_sentinel};}
private:
  std::vector<navi> _data;
  entry_type _entry;
};
