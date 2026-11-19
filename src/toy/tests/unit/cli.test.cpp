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
    auto constexpr ARGUMENTS = std::to_array({"toyc"});
    auto const options = parse_cli(ARGUMENTS);

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "-");
    EXPECT_EQ(options->options.emit, Action::None);
}

TEST_F(CliTest, ParsesInputFilename)
{
    auto constexpr ARGUMENTS = std::to_array({"toyc", "foo.toy"});
    auto const options = parse_cli(ARGUMENTS);

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "foo.toy");
    EXPECT_EQ(options->options.emit, Action::None);
}

TEST_F(CliTest, ParsesEmitAstOption)
{
    auto constexpr ARGUMENTS = std::to_array({"toyc", "foo.toy", "-emit=ast"});
    auto const options = parse_cli(ARGUMENTS);

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "foo.toy");
    EXPECT_EQ(options->options.emit, Action::DumpAST);
}

TEST_F(CliTest, ParsesEmitAstSeparateOption)
{
    auto constexpr ARGUMENTS = std::to_array({"toyc", "foo.toy", "-emit", "ast"});
    auto const options = parse_cli(ARGUMENTS);

    ASSERT_TRUE(options.has_value());
    EXPECT_EQ(options->input_filename, "foo.toy");
    EXPECT_EQ(options->options.emit, Action::DumpAST);
}

} // namespace
} // namespace toy
