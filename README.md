# Monorepo

Monorepo for my personal projects. Also serves as a demo for doing things the "right way". This always means using the most correct, most modern, most powerful tool for the job. There is no such thing as overkill.

Note: 5f3f3cf5b75c616d135399a14289dbb2a69f1db9 was the last commit with CMake/Conan.

## Initialize

*On the host*:

```fish
git clone https://github.com/rdong8/monorepo.git
cd monorepo/
```

Bootstrap the host:

```fish
just initialize-host
```

Then run `id` on the host to determine your user's UID and GID. Use that to fill in the `build.dockerfile.args.HOST_UID` and `build.dockerfile.args.HOST_GID` values in the [devcontainer.json](.devcontainer/devcontainer.json) file.

Then set the `dotfiles.repository` setting in VS Code to your dotfiles repository. Note that your install script MUST add Linuxbrew to the fish `PATH`.

Then build the devcontainer. All commands after this point are to be run *in the devcontainer*, not on the host.

## Development Workflow

Build and serve the doxygen documentation:

```fish
just docs
```

Register pre-commit hooks to run automatically:

```fish
just pre-commit-install
just pre-commit # Or manually run it
```

Clean the Bazel build directories (you shouldn't actually ever have to do this):

```fish
just clean
```
