module;

// Required for TableGen options struct declaration; see ToyOptions.hpp for details.
#include "src/toy/cli/ToyOptions.hpp"

export module toy:cli;

import std;

export namespace toy
{

using toy::Action;
using toy::ToyOptions;

struct CliOptions
{
    std::string input_filename{};
    ToyOptions options{};
};

[[nodiscard]]
auto parse_cli(int argument_count, char *argument_values[]) -> std::optional<CliOptions>;

} // namespace toy
