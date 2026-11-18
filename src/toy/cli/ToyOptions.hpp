#pragma once

/// @file ToyOptions.hpp
/// @brief TableGen option definitions header.
///
/// This header bridges TableGen's generated option parser with C++20 modules:
/// 1. TableGen's OPTIONS_STRUCT_DECL unconditionally emits `#include` directives
///    that cannot be placed within C++20 module purview.
/// 2. The generated struct definition requires pre-declaration of the `toy::Action` enum.
/// 3. Declaring the struct in a traditional header avoids symbol lookup ambiguities
///    when instantiating LLVM option templates across module units.

#include <cstdint>

namespace toy
{

enum class Action : std::uint8_t
{
    None,
    DumpAST,
};

} // namespace toy

#define OPTIONS_STRUCT_DECL
#include "ToyOptions.inc"
