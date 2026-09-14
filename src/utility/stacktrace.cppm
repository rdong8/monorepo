module;

#include <cpptrace/formatting.hpp>

export module utility:stacktrace;

export namespace utility
{

auto const trace_formatter = //
    cpptrace::formatter{}
        .colors(cpptrace::formatter::color_mode::always)
        .snippets(true)
        .snippet_context(3)
        .symbols(cpptrace::formatter::symbol_mode::pretty);

} // namespace utility
