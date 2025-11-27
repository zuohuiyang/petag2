#pragma once
#include <cstddef>

namespace base {

template <typename T>
class raw_span {
 public:
  using element_type = T;
  using pointer = const element_type*;
  using size_type = std::size_t;

  raw_span() : data_(nullptr), size_bytes_(0) {}
  raw_span(pointer data, size_type size_bytes) : data_(data), size_bytes_(size_bytes) {}
  template <typename U>
  raw_span(const base::span<U>& s) : data_(s.data()), size_bytes_(static_cast<size_type>(s.size())) {}

  pointer data() const { return data_; }
  size_type size_bytes() const { return size_bytes_; }
  size_type size() const { return size_bytes_; }

  template <typename U>
  raw_span& operator=(const base::span<U>& s) {
    data_ = s.data();
    size_bytes_ = static_cast<size_type>(s.size());
    return *this;
  }

 private:
  pointer data_;
  size_type size_bytes_;
};

}  // namespace base

