module;

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>

#include "src/toy/cli/ToyOptions.hpp"

module toy:cli.impl;

import std;

import :cli;

namespace cl = llvm::cl;

namespace
{

cl::opt<std::string> const input_filename{
    cl::Positional,
    cl::desc{"<input toy file>"},
    cl::init("-"),
    cl::value_desc{"filename"},
};

} // namespace

namespace toy
{

auto parse_cli(int argument_count, char *argument_values[]) -> std::optional<CliOptions>
{
    cl::LibraryOptionsContext context{};

    auto const success = cl::parseCommandLineOptions(
        context, llvm::ArrayRef(argument_values, static_cast<std::size_t>(argument_count)), "toy compiler\n");

    if (!success)
    {
        return std::nullopt;
    }

    return CliOptions{
        .input_filename{input_filename.getValue()},
        .options = context.get<ToyOptions>(),
    };
}

} // namespace toy
