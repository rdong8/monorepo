"""
Module extensions for LLVM/MLIR dependencies.
See:
- https://github.com/j2kun/mlir-tutorial/blob/main/extensions.bzl
- https://github.com/llvm/llvm-project/blob/main/utils/bazel/examples/http_archive/WORKSPACE
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _llvm_raw_impl(_module_context):
    COMMIT = "c82e0e904d688ace48b624422b2d6d85a83761ed"

    http_archive(
        name = "llvm-raw",
        build_file_content = "# empty",
        strip_prefix = "llvm-project-" + COMMIT,
        url =
            "https://github.com/rdong8/llvm-project/archive/{}.tar.gz".format(COMMIT),
    )

llvm_raw = module_extension(
    implementation = _llvm_raw_impl,
)
