module;

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>

// Required for TableGen options struct declaration; see ToyOptions.hpp for details.
#include "src/toy/cli/toy_options.hpp"

export module toy:cli;

import std;

namespace cl = llvm::cl;

namespace
{

cl::opt<std::string> const input_filename{
    cl::Positional,
    cl::desc{"<input toy file>"},
    cl::init("-"),
    cl::value_desc{"filename"},
};

}

namespace toy
{

export using toy::Action;
export using toy::ToyOptions;

export struct CliOptions
{
    std::string input_filename{};
    ToyOptions options{};
};

export [[nodiscard]]
auto parse_cli(std::span<char const *const> arguments) -> std::optional<CliOptions>
{
    cl::LibraryOptionsContext context{};

    auto const success = cl::parseCommandLineOptions(context, arguments, "toy compiler\n");

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
