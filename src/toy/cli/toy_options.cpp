#include "src/toy/cli/toy_options.hpp"

#include <llvm/Option/LibraryOptions.h>

#define OPTIONS_STRUCT_DEFS
#include "ToyOptions.inc"

LLVM_REGISTER_LIBRARY_OPTIONS(toy::ToyOptions)
