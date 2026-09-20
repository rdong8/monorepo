#!/usr/bin/env fish

brew bundle --file=(git rev-parse --show-toplevel)/brew/rust.Brewfile

mkdir -p ~/.cargo

# Setup wild linker and sccache
cat <<EOF >> ~/.cargo/config.toml
[build]
rustc-wrapper = "sccache"

[target.x86_64-unknown-linux-gnu]
linker = "clang"
rustflags = ["-Clink-arg=--ld-path=wild"]
EOF
