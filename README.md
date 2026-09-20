# Monorepo

Monorepo for my personal projects. Also serves as a demo for doing things the "right way". This always means using the most correct, most modern, most powerful tool for the job. There is no such thing as overkill.

## Initialize

*On the host*:

```fish
git clone https://github.com/rdong8/monorepo.git
cd monorepo/
```

WARNING: currently [devcontainer.json](.devcontainer/devcontainer.json) is set up to expect the repository to be cloned to `~/projects` on the host.

Bootstrap the host:

```fish
just initialize-host
```

Then run `id` on the host to determine your user's UID and GID. Use that to fill in the `build.dockerfile.args.HOST_UID` and `build.dockerfile.args.HOST_GID` values in the [devcontainer.json](.devcontainer/devcontainer.json) file.

Then set the `dotfiles.repository` setting in VS Code to your dotfiles repository. Note that your install script MUST add Linuxbrew to the fish `PATH`.

Then build the devcontainer. Wait until the terminal titled `Configuring...` completes (rebuilds should be quick).

All commands after this point are to be run *in the devcontainer*, not on the host.

## Development Workflow

See the [justfile](./justfile).

Register pre-commit hooks to run automatically:

```fish
just pre-commit-install
just pre-commit # Or manually run it
```

Install Bazel shell completions:

```fish
just bazel-completions
```

Install agent skills:

```fish
just skills
```

Build compile commands:

```fish
just compile_commands
```

Build and serve the doxygen documentation:

```fish
just docs
```

Tail last Bazel invocation:

```fish
just tail
```

Clean the Bazel build directories (you shouldn't actually ever have to do this):

```fish
just clean
```

## Rust

To develop non-monorepo Rust projects, run `scripts/setup_rust.fish`.
