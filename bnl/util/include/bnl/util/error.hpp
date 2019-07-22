#pragma once

#include <bnl/result.hpp>
#include <bnl/util/log.hpp>

#include <cstdlib>

namespace fmt {
template<>
struct formatter<bnl::status_code_domain::string_ref> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const bnl::status_code_domain::string_ref &string,
              FormatContext &ctx)
  {
    return format_to(
      ctx.out(), "{}", fmt::string_view(string.data(), string.size()));
  }
};
}

#define NOTREACHED()                                                           \
  LOG_E("Assertion failed: NOTREACHED()");                                     \
  std::abort();                                                                \
  (void) 0

#define TRY BNL_TRY

#define THROW(...)                                                             \
  {                                                                            \
    if (logger_) {                                                             \
      logger_->operator()(__FILE__,                                            \
                          static_cast<const char *>(__func__),                 \
                          __LINE__,                                            \
                          make_status_code(__VA_ARGS__));                      \
    }                                                                          \
                                                                               \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  (void) 0
