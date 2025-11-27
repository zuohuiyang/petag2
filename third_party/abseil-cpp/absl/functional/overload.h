#pragma once
#include <utility>

namespace absl {

template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
  explicit Overload(Ts... ts) : Ts(std::move(ts))... {}
};

}  // namespace absl

