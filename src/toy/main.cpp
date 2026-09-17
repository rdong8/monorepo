#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

import std;

import toy;

namespace cl = llvm::cl;

namespace
{

enum class Action : std::uint8_t
{
    None,
    DumpAST,
};

cl::opt<std::string> const input_filename{
    cl::Positional,
    cl::desc{"<input toy file>"},
    cl::init("-"),
    cl::value_desc{"filename"},
};

cl::opt<Action> const emit_action{
    "emit",
    cl::desc{"Select the kind of output desired"},
    cl::values(clEnumValN(Action::DumpAST, "ast", "output the AST dump")),
};

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
    cl::ParseCommandLineOptions(argc, argv, "toy compiler\n");

    auto const module_ast = parse_input_file(input_filename);

    if (!module_ast)
    {
        return 1;
    }

    switch (emit_action.getValue())
    {
        case Action::DumpAST:
        {
            toy::dump(*module_ast);
            return 0;
        }
        case Action::None:
        {
            toy::eprintln("No action specified (parsing only?), use -emit=<action>");
            return 1;
        }
    }
}
