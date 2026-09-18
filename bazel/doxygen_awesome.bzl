"""
https://github.com/jothepro/doxygen-awesome-css
"""

def _doxygen_awesome_repo_impl(repository_ctx):
    version = repository_ctx.attr.version

    repository_ctx.download_and_extract(
        url = "https://github.com/jothepro/doxygen-awesome-css/archive/refs/tags/v{}.tar.gz".format(version),
        integrity = repository_ctx.attr.integrity,
        stripPrefix = "doxygen-awesome-css-" + version,
    )

    css_files = sorted([
        p.basename
        for p in repository_ctx.path(".").readdir()
        if p.basename.endswith(".css") and "sidebar-only" not in p.basename
    ])
    js_files = sorted([p.basename for p in repository_ctx.path(".").readdir() if p.basename.endswith(".js")])

    repository_ctx.file(
        "bazel/defs.bzl",
        content = """\
CSS_FILES = {}
JS_FILES = {}
""".format(css_files, js_files),
    )

    repository_ctx.file("bazel/BUILD.bazel", "")

    repository_ctx.file(
        "BUILD.bazel",
        content = """\
load("//bazel:defs.bzl", "CSS_FILES", "JS_FILES")

exports_files(CSS_FILES + JS_FILES)

filegroup(
    name = "css",
    srcs = CSS_FILES,
    visibility = ["//visibility:public"],
)

filegroup(
    name = "js",
    srcs = JS_FILES,
    visibility = ["//visibility:public"],
)
""",
    )

doxygen_awesome_repo = repository_rule(
    implementation = _doxygen_awesome_repo_impl,
    attrs = {
        "integrity": attr.string(),
        "version": attr.string(mandatory = True),
    },
)

def _doxygen_awesome_impl(_module_context):
    doxygen_awesome_repo(
        name = "doxygen_awesome",
        version = "2.5.0",
    )

doxygen_awesome = module_extension(
    implementation = _doxygen_awesome_impl,
)
