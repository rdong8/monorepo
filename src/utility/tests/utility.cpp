module;

#include <cpptrace/cpptrace.hpp>
#include <gtest/gtest.h>

module utility:test;

import std;

import utility;

namespace utility
{
namespace
{

TEST(StacktraceTest, FormatterFormatsTrace)
{
    auto const trace = cpptrace::generate_trace();
    auto const formatted_trace = trace_formatter.format(trace);

    EXPECT_FALSE(formatted_trace.empty());
}

} // namespace
} // namespace utility
