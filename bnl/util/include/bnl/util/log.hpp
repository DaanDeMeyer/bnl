#pragma once

#include <bnl/log.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define LOG_T(format, ...) BNL_LOG_TRACE(logger_, format, ##__VA_ARGS__)
#define LOG_D(format, ...) BNL_LOG_DEBUG(logger_, format, ##__VA_ARGS__)
#define LOG_I(format, ...) BNL_LOG_INFO(logger_, format, ##__VA_ARGS__)
#define LOG_W(format, ...) BNL_LOG_WARNING(logger_, format, ##__VA_ARGS__)
#define LOG_E(format, ...) BNL_LOG_ERROR(logger_, format, ##__VA_ARGS__)

#pragma clang diagnostic pop
#pragma GCC diagnostic pop
