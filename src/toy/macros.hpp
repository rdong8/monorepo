#pragma once

/// @file macros.hpp
/// Macros shared across module partitions.

#include <cpptrace/cpptrace.hpp>
#include <llvm/Support/raw_ostream.h>

#define TOY_ASSERT_IMPL(expr, fmt, ...)                                                                                \
    [&]                                                                                                                \
    {                                                                                                                  \
        if (!(expr)) [[unlikely]]                                                                                      \
        {                                                                                                              \
            ::toy::eprintln(fmt __VA_OPT__(, ) __VA_ARGS__);                                                           \
            ::toy::eprintln("{}", ::utility::trace_formatter.format(cpptrace::generate_trace()));                      \
            std::terminate();                                                                                          \
        }                                                                                                              \
    }()

#define TOY_ASSERT(expr, fmt, ...)                                                                                     \
    TOY_ASSERT_IMPL(expr, "Assertion `{}` failed: " fmt, #expr __VA_OPT__(, ) __VA_ARGS__)

#define TOY_ASSERT_FALSE(fmt, ...)                                                                                     \
    TOY_ASSERT(false, fmt __VA_OPT__(, ) __VA_ARGS__);                                                                 \
    std::unreachable()

#define TOY_TODO(fmt, ...)                                                                                             \
    TOY_ASSERT_IMPL(false, "TODO:" fmt __VA_OPT__(, ) __VA_ARGS__);                                                    \
    std::unreachable()
