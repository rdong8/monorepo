module;

#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

module toy:utility.test;

import std;

import :utility;

namespace toy
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

} // namespace
} // namespace toy
