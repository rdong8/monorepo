module;

#include <cpptrace/cpptrace.hpp>
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

module utility:test;

import std;

import utility;

namespace utility
{
namespace
{

TEST(PrintTest, PrintFormatsCorrectly)
{
    std::string buffer{};
    llvm::raw_string_ostream stream{buffer};

    print(stream, "Hello, {}!", "world");

    EXPECT_EQ(buffer, "Hello, world!");
}

TEST(PrintTest, PrintlnAppendsNewline)
{
    std::string buffer{};
    llvm::raw_string_ostream stream{buffer};

    println(stream, "Value: {}", 42);

    EXPECT_EQ(buffer, "Value: 42\n");
}

TEST(StacktraceTest, FormatterFormatsTrace)
{
    auto const trace = cpptrace::generate_trace();
    auto const formatted_trace = trace_formatter.format(trace);

    EXPECT_FALSE(formatted_trace.empty());
}

} // namespace
} // namespace utility
