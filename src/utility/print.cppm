module;

#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>

export module utility:print;

import std;

export namespace utility
{

template <typename... Args>
auto print(llvm::raw_ostream &stream, std::string_view format_string, Args &&...arguments) -> void
{
    llvm::formatv(format_string.data(), std::forward<Args>(arguments)...).format(stream);
}

template <typename... Args> auto print(std::string_view format_string, Args &&...arguments) -> void
{
    print(llvm::outs(), format_string, std::forward<Args>(arguments)...);
}

template <typename... Args> auto eprint(std::string_view format_string, Args &&...arguments) -> void
{
    print(llvm::errs(), format_string, std::forward<Args>(arguments)...);
}

template <typename... Args>
auto println(llvm::raw_ostream &stream, std::string_view format_string, Args &&...arguments) -> void
{
    print(stream, format_string, std::forward<Args>(arguments)...);
    stream.write('\n');
}

template <typename... Args> auto println(std::string_view format_string, Args &&...arguments) -> void
{
    println(llvm::outs(), format_string, std::forward<Args>(arguments)...);
}

template <typename... Args> auto eprintln(std::string_view format_string, Args &&...arguments) -> void
{
    println(llvm::errs(), format_string, std::forward<Args>(arguments)...);
}

} // namespace utility
