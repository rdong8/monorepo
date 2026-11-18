#include "src/toy/cli/ToyOptions.hpp"

#include <llvm/Option/LibraryOptions.h>

#define OPTIONS_STRUCT_DEFS
#include "ToyOptions.inc"

LLVM_REGISTER_LIBRARY_OPTIONS(toy::ToyOptions)
