import os
import lit.formats
import lit.llvm
from lit.llvm.subst import ToolSubst

config.name = "toy"
config.test_format = lit.formats.ShTest()
config.suffixes = [".toy"]
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.environ.get("TEST_UNDECLARED_OUTPUTS_DIR", config.test_source_root)

lit.llvm.initialize(lit_config, config)
lit.llvm.llvm_config.use_default_substitutions()

tool_dirs = [config.toy_tools_dir, config.llvm_tools_dir]
for directory in tool_dirs:
    lit.llvm.llvm_config.with_environment("PATH", directory, append_path=True)

tools = [
    ToolSubst("toyc", os.path.join(config.toy_tools_dir, "toyc"), unresolved="fatal"),
    ToolSubst("FileCheck", os.path.join(config.llvm_tools_dir, "FileCheck"), unresolved="fatal"),
]

lit.llvm.llvm_config.add_tool_substitutions(tools, tool_dirs)
