# TODO: move to dedicated repo

FROM ubuntu:latest

# Basics / installation prerequisites
RUN apt update && apt install -y --no-install-recommends \
        gpg-agent debian-keyring \
        software-properties-common apt-utils \
        lsb-release software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update -y && \
    apt install -y \
        sudo tzdata openssh-server curl wget libssl-dev ca-certificates \
        less tar gzip zip unzip

# Remote
RUN apt install -y --no-install-recommends \
        openssh-server wget rsync ssh

# TODO: gdb, lldb

# C++ toolchain
ENV CLANG_VERSION=16
ENV GCC_VERSION=13

# GCC

    apt install -y  build-essential \
                    libc6  libc6-dev libstdc++6 \
                    gcc-${GCC_VERSION} g++-${GCC_VERSION} \
                    gdb
RUN update-alternatives --install /usr/bin/gcc  gcc  /usr/bin/gcc-${GCC_VERSION} 100    \
                        --slave   /usr/bin/g++  g++  /usr/bin/g++-${GCC_VERSION}        \
                        --slave   /usr/bin/gcov gcov /usr/bin/gcov-${GCC_VERSION}

# LLVM
RUN apt install -y wget bash && \
    wget https://apt.llvm.org/llvm.sh && \  
    chmod +x llvm.sh && \
    (yes '' | ./llvm.sh $CLANG_VERSION all) && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$CLANG_VERSION 100     \
    --slave /usr/bin/clang++         clang++         /usr/bin/clang++-$CLANG_VERSION         \
    --slave /usr/bin/clang-format    clang-format    /usr/bin/clang-format-$CLANG_VERSION    \
    --slave /usr/bin/clang-tidy      clang-tidy      /usr/bin/clang-tidy-$CLANG_VERSION      \
    --slave /usr/bin/clangd          clangd          /usr/bin/clangd-$CLANG_VERSION          \
    --slave /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-$CLANG_VERSION

# Tooling
RUN apt upgrade -y && \
    apt install -y  wget                \
                    make cmake ninja-build    \
                    libboost-all-dev    \ 
                    # TODO: external
                    git                 \ 
                    graphviz        

# Tooling: Doxygen

# Tooling: zsh
RUN apt update && apt upgrade -y && \
    apt install -y zsh && \
    yes '' | sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)"

# CMD ["zsh"]

# Cleanup

