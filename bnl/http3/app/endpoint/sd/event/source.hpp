#pragma once

#include <os/result.hpp>
#include <sd/event/time.hpp>

#include <cstdint>
#include <functional>
#include <memory>

using namespace bnl;

struct sd_event_source;

namespace sd {
namespace event {

class loop;

class source {
public:
  source(source &&other) noexcept;
  source &operator=(source &&other) noexcept;

  operator bool() const noexcept;

protected:
  source(sd_event_source *source) noexcept;

  sd_event_source *sd() const noexcept;

private:
  std::unique_ptr<sd_event_source, sd_event_source *(*) (sd_event_source *)>
    source_;
};

class io : public source {
public:
  using handler = std::function<result<void>(uint32_t)>;

  io(io &&) = default;
  io &operator=(io &&) = default;

  void on_io(handler handler) noexcept;

private:
  friend class loop;

  io(sd_event_source *source) noexcept;

  static int handle(sd_event_source *source,
                    int fd,
                    uint32_t revents,
                    void *context);

private:
  handler on_io_;
};

class timer : public source {
public:
  using handler = std::function<result<void>(sd::event::duration)>;

  timer(timer &&) = default;
  timer &operator=(timer &&) = default;

  void update(duration usec);

  void on_expire(handler handler) noexcept;

private:
  friend class loop;

  timer(sd_event_source *source) noexcept;

  static int handle(sd_event_source *source, uint64_t usec, void *context);

private:
  handler on_expire_;
};

}
}
