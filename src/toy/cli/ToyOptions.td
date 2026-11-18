include "llvm/Option/LibraryOptions.td"

def ToyOptions : OptionsStruct<"toy">;

defm : EnumField<"emit", "toy::Action", "toy::Action::None", "ast",
                 ["toy::Action::DumpAST"], "Select the kind of output desired">;
