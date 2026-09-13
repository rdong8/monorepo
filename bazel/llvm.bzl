"""
Module extensions for LLVM/MLIR dependencies.
See:
- https://github.com/j2kun/mlir-tutorial/blob/main/extensions.bzl
- https://github.com/llvm/llvm-project/blob/main/utils/bazel/examples/http_archive/WORKSPACE
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _llvm_raw_deps_impl(_module_context):
    LLVM_COMMIT = "2fbf0823ee4c74a779c9f31bc4d23c99d7444670"

    http_archive(
        name = "llvm-raw",
        build_file_content = "# empty",
        strip_prefix = "llvm-project-" + LLVM_COMMIT,
        url =
            "https://github.com/rdong8/llvm-project/archive/{}.tar.gz".format(LLVM_COMMIT),
    )

llvm_raw_deps = module_extension(
    implementation = _llvm_raw_deps_impl,
)
