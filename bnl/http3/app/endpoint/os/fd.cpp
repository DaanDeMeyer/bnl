#include <os/fd.hpp>

namespace os {

fd::fd(int fd) noexcept
  : fd_(fd)
{}

fd::fd(fd &&other) noexcept
  : fd(other.fd_)
{
  other.fd_ = -1;
}

fd &
fd::operator=(fd &&other) noexcept
{
  if (&other != this) {
    fd_ = other.fd_;
    other.fd_ = -1;
  }

  return *this;
}

fd::~fd() noexcept
{
  if (fd_ != -1) {
    close(fd_);
  }
}

fd::operator int() const noexcept
{
  return fd_;
}

bool
operator!=(const fd &lhs, const fd &rhs)
{
  return !(rhs == lhs);
}

}
