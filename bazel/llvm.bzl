"""
Module extensions for LLVM/MLIR dependencies.
See:
- https://github.com/j2kun/mlir-tutorial/blob/main/extensions.bzl
- https://github.com/llvm/llvm-project/blob/main/utils/bazel/examples/http_archive/WORKSPACE
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def _llvm_raw_deps_impl(_module_context):
    new_git_repository(
        name = "llvm-raw",
        build_file_content = "",
        commit = "04f2983e41790a2f1ec1f073f528ceccb212210a",
        init_submodules = False,
        remote = "https://github.com/rdong8/llvm-project.git",
    )

llvm_raw_deps = module_extension(
    implementation = _llvm_raw_deps_impl,
)
