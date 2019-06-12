#pragma once

#include <h3c/buffer.hpp>

#include <deque>

namespace h3c {

class H3C_EXPORT buffers {
public:
  buffers() = default;

  class anchor;
  class discarder;

  size_t size() const noexcept;
  bool empty() const noexcept;

  uint8_t operator[](size_t index) const noexcept;
  uint8_t operator*() const noexcept;

  buffer slice(size_t size) const;

  void push(buffer buffer);

  void consume(size_t size) noexcept;
  buffers &operator+=(size_t size) noexcept;

  size_t consumed() const noexcept;

  void undo(size_t size) noexcept;

  void discard();

private:
  std::deque<buffer> buffers_;

  buffer concat(size_t start, size_t end, size_t left) const;
};

class H3C_EXPORT buffers::anchor {
public:
  explicit anchor(buffers &buffers) noexcept;

  H3C_NO_COPY(anchor);
  H3C_NO_MOVE(anchor);

  void relocate() noexcept;

  void release() noexcept;

  ~anchor() noexcept;

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
