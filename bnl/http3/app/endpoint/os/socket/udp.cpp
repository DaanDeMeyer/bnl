#include <os/socket/udp.hpp>

#include <os/error.hpp>

#include <bnl/base/error.hpp>
#include <bnl/util/error.hpp>

#include <arpa/inet.h>
#include <cassert>
#include <sys/epoll.h>
#include <sys/ioctl.h>

static sockaddr_storage
make_sockaddr(ip::endpoint endpoint)
{
  sockaddr_storage storage = {};
  storage.ss_family =
    endpoint.address() == ip::address::type::ipv4 ? AF_INET : AF_INET6;

  base::buffer_view address = endpoint.address().bytes();
  uint16_t port = endpoint.port();

  switch (endpoint.address()) {
    case ip::address::type::ipv4: {
      sockaddr_in ipv4 = {};

      ipv4.sin_family = AF_INET;
      memcpy(&ipv4.sin_addr.s_addr, address.data(), address.size());
      ipv4.sin_port = htons(port);

      memcpy(&storage, &ipv4, sizeof(ipv4));
      break;
    }

    case ip::address::type::ipv6: {
      sockaddr_in6 ipv6 = {};

      ipv6.sin6_family = AF_INET6;
      memcpy(&ipv6.sin6_addr, address.data(), address.size());
      ipv6.sin6_port = htons(port);

      memcpy(&storage, &ipv6, sizeof(ipv6));
      break;
    }
  }

  return storage;
}

static ip::endpoint
make_endpoint(const sockaddr *sockaddr)
{
  assert(sockaddr != nullptr);

  switch (sockaddr->sa_family) {
    case AF_INET: {
      const sockaddr_in *ipv4 = reinterpret_cast<const sockaddr_in *>(sockaddr);

      ipv4::address ip(ipv4->sin_addr.s_addr);
      uint16_t port = ntohs(ipv4->sin_port);

      return { ip, port };
    }

    case AF_INET6: {
      const sockaddr_in6 *ipv6 =
        reinterpret_cast<const sockaddr_in6 *>(sockaddr);

      const uint8_t *bytes =
        reinterpret_cast<const uint8_t *>(ipv6->sin6_addr.s6_addr);

      ipv6::address ip({ bytes, ipv6::address::size });
      uint16_t port = ntohs(ipv6->sin6_port);

      return { ip, port };
    }
  }

  assert(false);
  return {};
}

static result<os::fd>
make_socket(ip::endpoint peer, const log::api *logger_)
{
  sockaddr_storage storage = make_sockaddr(peer);

  static constexpr int type =
    SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC; // NOLINT

  os::fd fd(::socket(storage.ss_family, type, IPPROTO_UDP));
  if (fd == -1) {
    THROW_SYSTEM(socket, errno);
  }

  int rv =
    ::connect(fd, reinterpret_cast<sockaddr *>(&storage), sizeof(storage));
  if (rv == -1) {
    THROW_SYSTEM(connect, errno);
  }

  static constexpr int reuseaddr = 1;
  rv = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
  if (rv == -1) {
    THROW_SYSTEM(setsocktop, errno);
  }

  return success(std::move(fd));
}

namespace os {
namespace socket {

udp::udp(ip::endpoint peer, const log::api *logger)
  : socket_(make_socket(peer, logger).assume_value())
  , logger_(logger)
{}

ip::endpoint
udp::local() const
{
  sockaddr_storage storage = {};
  socklen_t size = sizeof(storage);

  int rv = getsockname(socket_, reinterpret_cast<sockaddr *>(&storage), &size);
  if (rv == -1) {
    assert(errno == ENOBUFS);
    throw std::bad_alloc();
  }

  return make_endpoint(reinterpret_cast<sockaddr *>(&storage));
}

ip::endpoint
udp::peer() const
{
  sockaddr_storage storage = {};
  socklen_t size = sizeof(storage);

  int rv = getpeername(socket_, reinterpret_cast<sockaddr *>(&storage), &size);
  if (rv == -1) {
    assert(errno == ENOBUFS);
    throw std::bad_alloc();
  }

  return make_endpoint(reinterpret_cast<sockaddr *>(&storage));
}

const os::fd &
udp::fd() const noexcept
{
  return socket_;
}

system_code
udp::error() const noexcept
{
  int error = 0;
  socklen_t size = sizeof(error);

  int rv = getsockopt(socket_, SOL_SOCKET, SO_ERROR, &error, &size);
  if (rv == -1) {
    THROW_SYSTEM(getsockopt, errno);
  }

  return posix_code(errno);
}

result<void>
udp::send()
{
  if (send_buffer_.empty()) {
    return base::error::idle;
  }

  const base::buffer &first = send_buffer_.front();
  ssize_t rv = ::send(socket_, first.data(), first.size(), 0);

  if (rv == -1) {
    if (errno == EAGAIN) {
      return posix_code(errno);
    }

    THROW_SYSTEM(write, errno);
  }

  send_buffer_.consume(static_cast<size_t>(rv));

  LOG_T("send: {}", rv);

  return success();
}

void
udp::add(base::buffer buffer)
{
  send_buffer_.push(std::move(buffer));
}

result<base::buffer>
udp::recv()
{
  int size = 0;

  if ((ioctl(socket_, FIONREAD, &size)) == -1) { // NOLINT
    THROW_SYSTEM(ioctl, errno);
  }

  base::buffer datagram(static_cast<size_t>(size));

  ssize_t rv = ::recv(socket_, datagram.data(), datagram.size(), 0);
  if (rv == -1) {
    if (errno == EAGAIN) {
      return posix_code(errno);
    }

    THROW_SYSTEM(read, errno);
  }

  LOG_T("recv: {}", rv);

  return datagram;
}

}
}
