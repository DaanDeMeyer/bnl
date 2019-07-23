#pragma once

#include <unistd.h>

namespace os {

class fd {
public:
  fd() = default;
  explicit fd(int fd) noexcept;

  fd(fd &&other) noexcept;
  fd &operator=(fd &&other) noexcept;

  ~fd() noexcept;

  operator int() const noexcept;

private:
  int fd_ = -1;
};

}
