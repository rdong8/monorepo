module;

#include <gtest/gtest.h>
#include <llvm/Support/CommandLine.h>

module toy:cli.test;

import std;

import :cli;

namespace toy
{
namespace
{

class CliTest : public ::testing::Test
{
  protected:
    auto SetUp() -> void override
    {
        llvm::cl::ResetAllOptionOccurrences();
    }

    auto TearDown() -> void override
    {
        llvm::cl::ResetAllOptionOccurrences();
    }
};

TEST_F(CliTest, ParsesDefaultOptions)
{
    std::array<char const *, 1> const arguments{"toyc"};

    auto const options = parse_cli(static_cast<int>(arguments.size()), const_cast<char **>(arguments.data()));

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "-");
    EXPECT_EQ(options->options.emit, Action::None);
}

TEST_F(CliTest, ParsesInputFilename)
{
    std::array<char const *, 2> const arguments{"toyc", "foo.toy"};

    auto const options = parse_cli(static_cast<int>(arguments.size()), const_cast<char **>(arguments.data()));

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "foo.toy");
    EXPECT_EQ(options->options.emit, Action::None);
}

TEST_F(CliTest, ParsesEmitAstOption)
{
    std::array<char const *, 3> const arguments{"toyc", "foo.toy", "-emit=ast"};

    auto const options = parse_cli(static_cast<int>(arguments.size()), const_cast<char **>(arguments.data()));

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "foo.toy");
    EXPECT_EQ(options->options.emit, Action::DumpAST);
}

TEST_F(CliTest, ParsesEmitAstSeparateOption)
{
    std::array<char const *, 4> const arguments{"toyc", "foo.toy", "-emit", "ast"};

    auto const options = parse_cli(static_cast<int>(arguments.size()), const_cast<char **>(arguments.data()));

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "foo.toy");
    EXPECT_EQ(options->options.emit, Action::DumpAST);
}

} // namespace
} // namespace toy
