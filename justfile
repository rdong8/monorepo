set dotenv-load
set shell := ['fish', '-c']

export UBSAN_OPTIONS := 'print_stacktrace=1'

configs := ''
targets := ''

initialize-host:
    sudo dnf install cpp podman

# TODO: The only reason we have a .bazelversion is because fish completions can't be generated without it
# https://github.com/bazelbuild/bazelisk/issues/718#issuecomment-3435688542
[doc]
bazel-completions:
    bazelisk completion fish > ~/.config/fish/completions/bazelisk.fish

skills:
    gh skill install intel/intel-performance-skills --scope user

[private]
bazel cmd *targets=targets:
    bazel \
        {{ cmd }} \
        {{ configs }} \
        {{ targets }}

build *targets=targets: (bazel "build" targets)

test *targets=targets: (bazel "test" targets)

run target=targets: (bazel "run" target)

# HACK: hedron_compile_commands doesn't support C++20 modules, have to build first
[doc]
compile_commands: (build "//src/...") (run "@hedron_compile_commands//:refresh_all")

docs: (run "//docs:serve")

[doc("Tail the last Bazel invocation")]
tail *args='-F':
    #!/usr/bin/env fish
    set -l log (test -L bazel-out && readlink -f bazel-out/../../../command.log 2>/dev/null)
    if test (count $log) -eq 0
        set -l cache_dir (test -n "$XDG_CACHE_HOME" && echo "$XDG_CACHE_HOME" || echo "$HOME/.cache")
        set -l hash (echo -n '{{ justfile_directory() }}' | md5sum | cut -d' ' -f1)
        set log "$cache_dir/bazel/_bazel_$USER/$hash/command.log"
    end
    tail {{ args }} $log

pre-commit:
    prek run --all-files

pre-commit-install:
    prek install

pre-commit-update:
    prek autoupdate

clean *args:
    bazel clean {{ args }}

update-submodules:
    git submodule update --init --recursive --remote
