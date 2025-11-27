#pragma once
#include <cstddef>

namespace base {

template <typename T>
class span {
 public:
  using element_type = T;
  using pointer = const element_type*;
  using size_type = std::size_t;

  span() : data_(nullptr), size_(0) {}
  span(pointer data, size_type size) : data_(data), size_(size) {}
  template <typename Container>
  span(const Container& c) : data_(c.data()), size_(static_cast<size_type>(c.size())) {}

  pointer data() const { return data_; }
  size_type size() const { return size_; }

  const element_type* begin() const { return data_; }
  const element_type* end() const { return data_ + size_; }

 private:
  pointer data_;
  size_type size_;
};

}  // namespace base

