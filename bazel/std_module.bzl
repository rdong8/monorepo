"""
Provides the C++23 standard library module repository rule and module extension.
See: https://github.com/igormcoelho/rules_cpp23_modules/tree/main/demo9
"""

def _std_module_repo_impl(rctx):
    llvm_build_file = rctx.path(rctx.attr.llvm_repo)
    libcxx_dir = llvm_build_file.dirname.get_child("share").get_child("libc++").get_child("v1")

    if not libcxx_dir.exists:
        fail("libc++ module directory not found at: %s" % libcxx_dir)

    rctx.symlink(libcxx_dir.get_child("std.cppm"), "std.cppm")
    rctx.symlink(libcxx_dir.get_child("std"), "std")

    rctx.file(
        "BUILD.bazel",
        content = """\
load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "std",
    srcs = glob(["std/*.inc"]),
    module_interfaces = ["std.cppm"],
    copts = ["-Wno-reserved-module-identifier"],
)

cc_library(
    name = "std_module",
    deps = [":std"],
)
""",
        executable = False,
    )

std_module_repo = repository_rule(
    implementation = _std_module_repo_impl,
    attrs = {
        "llvm_repo": attr.label(default = Label("@llvm_toolchain_llvm//:BUILD.bazel")),
    },
)

def _std_module_extension_impl(_mctx):
    std_module_repo(name = "std_module")

std_module = module_extension(
    implementation = _std_module_extension_impl,
)
