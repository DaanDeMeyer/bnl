#pragma once

#include <h3c/log.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define LOG_T(format, ...) H3C_LOG_TRACE(logger_, format, ##__VA_ARGS__)
#define LOG_D(format, ...) H3C_LOG_DEBUG(logger_, format, ##__VA_ARGS__)
#define LOG_I(format, ...) H3C_LOG_INFO(logger_, format, ##__VA_ARGS__)
#define LOG_W(format, ...) H3C_LOG_WARNING(logger_, format, ##__VA_ARGS__)
#define LOG_E(format, ...) H3C_LOG_ERROR(logger_, format, ##__VA_ARGS__)

#pragma clang diagnostic pop
#pragma GCC diagnostic pop
