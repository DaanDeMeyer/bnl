#include <sd/event/loop.hpp>

#include <os/error.hpp>
#include <os/fd.hpp>

#include <systemd/sd-event.h>

#include <exception>

namespace sd {

namespace event {

loop::loop()
  : event_(nullptr, sd_event_unref)

{
  sd_event *event = nullptr;

  int rv = sd_event_default(&event);
  if (rv < 0) {
    throw std::bad_alloc();
  }

  event_ = decltype(event_)(event, sd_event_unref);
}

duration
loop::now() const noexcept
{
  uint64_t result = 0;

  int rv = sd_event_now(event_.get(), CLOCK_MONOTONIC, &result);
  (void) rv; // Silence GCC 9 Wunused-variable warning
  assert(rv >= 0);

  return duration(result);
}

std::function<result<duration>()>
loop::clock() const noexcept
{
  return [this]() -> result<duration> { return now(); };
}

result<io>
loop::io(const os::fd &fd)
{
  sd_event_source *source = nullptr;
  static constexpr uint32_t events = EPOLLIN | EPOLLOUT | EPOLLET;

  int rv =
    sd_event_add_io(event_.get(), &source, fd, events, io::handle, nullptr);
  if (rv < 0) {
    THROW_SYSTEM(sd_event_add_io, -rv);
  }

  return event::io(source);
}

result<timer>
loop::timer()
{
  sd_event_source *source = nullptr;

  int rv = sd_event_add_time(event_.get(),
                             &source,
                             CLOCK_MONOTONIC,
                             UINT64_MAX,
                             1,
                             timer::handle,
                             nullptr);
  if (rv < 0) {
    THROW_SYSTEM(sd_event_add_time, -rv);
  }

  return event::timer(source);
}

result<void>
loop::signal(int signal)
{
  sigset_t sigset;

  int rv = sigemptyset(&sigset);
  if (rv == -1) {
    THROW_SYSTEM(sigemptyset, errno);
  }

  rv = sigaddset(&sigset, signal);
  if (rv == -1) {
    THROW_SYSTEM(sigaddset, errno);
  }

  rv = sigprocmask(SIG_BLOCK, &sigset, nullptr);
  if (rv == -1) {
    THROW_SYSTEM(sigprocmask, errno);
  }

  rv = sd_event_add_signal(event_.get(), nullptr, signal, nullptr, nullptr);
  if (rv < 0) {
    THROW_SYSTEM(sd_event_add_signal, -rv);
  }

  return success();
}

result<void>
loop::run()
{
  int rv = sd_event_loop(event_.get());

  if (!error_.empty()) {
    return error_.clone();
  }

  if (rv != 0) {
    THROW_SYSTEM(sd_event_loop, -rv);
  }

  return success();
}

result<void>
loop::exit()
{
  int rv = sd_event_exit(event_.get(), 0);
  if (rv < 0) {
    THROW_SYSTEM(sd_event_exit, -rv);
  }

  return success();
}

result<void>
loop::exit(system_code sc)
{
  int rv = sd_event_exit(event_.get(), static_cast<int>(sc.value()));
  if (rv < 0) {
    THROW_SYSTEM(sd_event_exit, -rv);
  }

  error_ = std::move(sc);
  return success();
}

}
}
