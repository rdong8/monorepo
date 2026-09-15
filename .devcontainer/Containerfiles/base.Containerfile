##// Note that this file will be put through the C preprocessor, so you need to use ##// for comments

FROM docker.io/fedora:latest

##// For now, only install the following with the system package manager:
##// - Dependencies of linuxbrew itself (@development-tools, curl, file, procps-ng)
##// - gcc-c++: system toolchain needed for linuxbrew's LLVM
##// - fish: need to set user's shell when we create it
##// - stow: to install dotfiles
##// - which: to find fish for the useradd
##// - mathjax: dependency of doxygen, but not available via linuxbrew
RUN <<EOF
  dnf update -y
  dnf install -y \
    @development-tools \
    curl \
    file \
    fish \
    gcc-c++ \
    mathjax \
    procps-ng \
    stow \
    which
  dnf clean all
  rm -rf /var/cache/dnf
EOF

##// Add user
ARG REMOTE_USER
ARG HOST_UID
ARG HOST_GID
RUN <<EOF
  groupadd --gid ${HOST_GID} ${REMOTE_USER}
  useradd -ms $(which fish) --uid ${HOST_UID} --gid ${HOST_GID} ${REMOTE_USER}
EOF
USER ${REMOTE_USER}
##// Ensure ~/.cache is owned by REMOTE_USER so volume mounts in ~/.cache don't create it as root:root
RUN mkdir -p /home/${REMOTE_USER}/.cache
ARG FISH_CONFIG=/home/${REMOTE_USER}/.config/fish

##// Get homebrew binary
ARG HOMEBREW_PREFIX=/home/linuxbrew/.linuxbrew
COPY \
  --from=ghcr.io/homebrew/brew:latest \
  --chown=${REMOTE_USER} \
  ${HOMEBREW_PREFIX} \
  ${HOMEBREW_PREFIX}

ARG BREWFILE
RUN cat <<EOF >${BREWFILE}
  brew "bat"
  brew "btop"
  brew "cloc"
  brew "eza"
  brew "fastfetch"
  brew "fd"
  brew "fzf"
  brew "gawk"
  brew "gh"
  brew "git-delta"
  brew "helix"
  brew "jq"
  brew "just"
  brew "lldb"
  brew "prek"
  brew "ripgrep"
  brew "terror/tap/just-lsp", trusted: true
  brew "trash-cli"
  brew "tree"
  brew "wild-linker/wild/wild", trusted: true
  brew "wget"
  brew "zellij"
EOF
