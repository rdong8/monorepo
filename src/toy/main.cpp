#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>

import std;

import toy;

namespace
{

[[nodiscard]]
auto parse_input_file(llvm::StringRef filename) -> toy::ASTPtr<toy::ModuleAST>
{
    auto const file_or_error = llvm::MemoryBuffer::getFileOrSTDIN(filename);

    if (auto const error_code = file_or_error.getError())
    {
        toy::eprintln("Could not open input file: {}", error_code.message());
        return nullptr;
    }

    auto const buffer = file_or_error.get()->getBuffer();
    toy::LexerBuffer lexer{buffer.begin(), buffer.end(), std::string{filename}};
    toy::Parser parser{lexer};

    return parser.parse_module();
}

} // namespace

auto main(int argc, char *argv[]) -> int
{
    auto const cli_options = toy::parse_cli(argc, argv);

    if (!cli_options)
    {
        return 1;
    }

    auto const module_ast = parse_input_file(cli_options->input_filename);

    if (!module_ast)
    {
        return 1;
    }

    switch (cli_options->options.emit)
    {
        case toy::Action::DumpAST:
        {
            toy::dump(*module_ast);
            return 0;
        }
        case toy::Action::None:
        {
            toy::eprintln("No action specified (parsing only?), use -emit=<action>");
            return 1;
        }
    }
}
