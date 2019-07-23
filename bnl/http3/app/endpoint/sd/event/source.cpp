#include <sd/event/source.hpp>

#include <systemd/sd-event.h>

#include <cassert>

namespace sd {
namespace event {

source::source(sd_event_source *source) noexcept
  : source_(source, sd_event_source_unref)
{
  sd_event_source_set_userdata(source, this);
}

source::source(source &&other) noexcept
  : source_(nullptr, nullptr)
{
  operator=(std::move(other));
}

source &
source::operator=(source &&other) noexcept
{
  source_ = std::move(other.source_);

  sd_event_source_set_userdata(source_.get(), this);

  return *this;
}

sd_event_source *
source::sd() const noexcept
{
  return source_.get();
}

source::operator bool() const noexcept
{
  return source_ == nullptr;
}

io::io(sd_event_source *source) noexcept
  : source::source(source)
{}

void
io::on_io(handler handler) noexcept
{
  on_io_ = std::move(handler);
}

int
io::handle(sd_event_source *source, int fd, uint32_t revents, void *context)
{
  (void) source;
  (void) fd;
  auto io = static_cast<event::io *>(context);

  assert(source == io->sd());

  result<void> r = io->on_io_(revents);

  return r ? 0 : -1;
}

timer::timer(sd_event_source *source) noexcept
  : source::source(source)
{
  sd_event_source_set_userdata(source, this);
}

void
timer::update(duration usec)
{
  int rv = sd_event_source_set_time(sd(), usec.count());
  if (rv < 0) {
    assert(rv == ENOMEM);
    throw std::bad_alloc();
  }

  rv = sd_event_source_set_enabled(sd(), SD_EVENT_ONESHOT);
  if (rv < 0) {
    assert(rv == ENOMEM);
    throw std::bad_alloc();
  }
}

void
timer::on_expire(handler handler) noexcept
{
  on_expire_ = std::move(handler);
}

int
timer::handle(sd_event_source *source, uint64_t usec, void *context)
{
  (void) source;

  auto time = static_cast<event::timer *>(context);

  assert(source == time->sd());

  result<void> r = time->on_expire_(duration(usec));

  return r ? 0 : -1;
}

}
}
