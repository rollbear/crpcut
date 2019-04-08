#ifndef CRPCUT_SPLIT_RANGE_HPP
#define CRPCUT_SPLIT_RANGE_HPP

#include <algorithm>

namespace crpcut {
  template <typename I>
  struct iterator_range
  {
    I b;
    I e;
    constexpr I begin() const { return b;}
    constexpr I end() const { return e; }
  };

  template <typename I, typename E, typename P>
  class split_range_iterator
  {
  public:
    constexpr split_range_iterator(I i_, E e_, P p_)
      : i{i_}, n{std::find_if(i_, e_, p_)}, e{e_}, p{p_}
    {
    }
    constexpr iterator_range<I> operator*() const { return {i,n};}
    constexpr I begin() const { return i; }
    constexpr I end() const { return n; }
    constexpr split_range_iterator& operator++()
    {
      i = n == e ? e : std::next(n);
      n = std::find_if(i,e,p);
      return *this;
    }
    constexpr split_range_iterator operator++(int)
    {
      auto copy = *this;
      ++*this;
      return copy;
    }
    constexpr bool operator==(const split_range_iterator& other) const
    {
      return i == other.i && n == other.n && e == other.e;
    }
    constexpr bool operator!=(const split_range_iterator& other) const
    {
      return !(*this == other);
    }
  private:
    I i;
    I n;
    E e;
    P p;
  };

  template <typename I, typename E, typename P>
  class split_range_if
  {
  public:
    constexpr split_range_if(I i_, E e_, P p_) : i{i_}, e{e_}, p{p_} {}
    constexpr auto begin() const { return split_range_iterator(i,e,p); }
    constexpr auto end() const { return split_range_iterator(e,e,p); }
  private:
    I i;
    E e;
    P p;
  };

  template <typename I, typename E, typename D>
  constexpr auto split_range(I i, E e, D d)
  {
    return split_range_if(i,e, [d](auto x) { return x == d; });
  }

}
#endif //CRPCUT_SPLIT_RANGE_HPP
