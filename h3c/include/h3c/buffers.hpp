#pragma once

#include <h3c/buffer.hpp>

#include <deque>

namespace h3c {

class buffers {
public:
  buffers() = default;

  class anchor;
  class discarder;

  H3C_EXPORT size_t size() const noexcept;
  H3C_EXPORT bool empty() const noexcept;

  H3C_EXPORT uint8_t operator[](size_t index) const noexcept;
  H3C_EXPORT uint8_t operator*() const noexcept;

  H3C_EXPORT buffer slice(size_t size) const;

  H3C_EXPORT void push(buffer buffer);

  H3C_EXPORT void consume(size_t size) noexcept;
  H3C_EXPORT buffers &operator+=(size_t size) noexcept;

  H3C_EXPORT size_t consumed() const noexcept;

  H3C_EXPORT void undo(size_t size) noexcept;

  H3C_EXPORT void discard();

private:
  std::deque<buffer> buffers_;

  buffer concat(size_t start, size_t end, size_t left) const;
};

class buffers::anchor {
public:
  H3C_EXPORT explicit anchor(buffers &buffers) noexcept;

  H3C_NO_COPY(anchor);
  H3C_NO_MOVE(anchor);

  H3C_EXPORT void relocate() noexcept;

  H3C_EXPORT void release() noexcept;

  H3C_EXPORT ~anchor() noexcept;

private:
  buffers &buffers_;

  bool released_ = false;
  size_t position_ = 0;
};

class buffers::discarder {
public:
  explicit discarder(buffers &buffers) noexcept;

  H3C_NO_COPY(discarder);
  H3C_NO_MOVE(discarder);

  ~discarder() noexcept;

private:
  buffers &buffers_;
};

} // namespace h3c
