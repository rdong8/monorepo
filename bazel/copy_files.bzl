"""
Rule to copy or symlink multiple files into the current package.
"""

def _copy_files_impl(ctx):
    outs = []

    for src in ctx.files.srcs:
        out = ctx.actions.declare_file(src.basename)

        if ctx.attr.allow_symlink:
            ctx.actions.symlink(
                output = out,
                target_file = src,
            )
        else:
            ctx.actions.run_shell(
                inputs = [src],
                outputs = [out],
                command = 'cp -f "$1" "$2"',
                arguments = [src.path, out.path],
                mnemonic = "CopyFiles",
                use_default_shell_env = True,
            )

        outs.append(out)

    return [DefaultInfo(files = depset(outs))]

copy_files = rule(
    doc = "Stages multiple files or filegroups into the package directory.",
    implementation = _copy_files_impl,
    attrs = {
        "allow_symlink": attr.bool(default = True, doc = "Whether to allow symlinking instead of copying."),
        "srcs": attr.label_list(allow_files = True, mandatory = True, doc = "Files or filegroups to stage into this package."),
    },
)
