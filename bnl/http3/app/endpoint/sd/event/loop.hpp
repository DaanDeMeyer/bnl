#pragma once

#include <sd/event/source.hpp>
#include <sd/event/time.hpp>

#include <bnl/result.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

using namespace bnl;

struct sd_event;

namespace os {
class fd;
}

namespace sd {
namespace event {

class loop {
public:
  loop();

  loop(loop &&) = default;
  loop &operator=(loop &&) = default;

  duration now() const noexcept;
  std::function<result<duration>()> clock() const noexcept;

  result<event::io> io(const os::fd &fd);
  result<event::timer> timer();
  result<void> signal(int signal);

  result<void> run();

  result<void> exit();
  result<void> exit(system_code sc);

private:
  std::unique_ptr<sd_event, sd_event *(*) (sd_event *)> event_;
  system_code error_;
};

}
}
