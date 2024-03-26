# TODO: move to dedicated repo
# TODO: build image on weekly basis

FROM ubuntu:latest

ENV DEBIAN_FRONTEND noninteractive

# Basics / installation prerequisites
RUN apt update && apt install -y --no-install-recommends \
        gpg-agent debian-keyring \
        software-properties-common apt-utils \
        lsb-release software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update -y && \
    apt install -y \
        # remote
        openssh-server wget rsync ssh \
        sudo tzdata openssh-server curl libssl-dev ca-certificates \
        less tar zip unzip gzip \
        build-essential pkg-config \
        # build
        make cmake ninja-build \
        # versioning
        git subversion \
        # libraries
        libboost-all-dev \
        # tools
        doxygen graphviz \
        emacs nano vim

# vcpkg
RUN wget -qO vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz && \
    mkdir /opt/vcpkg && \
    tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh && \
    ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg

# Tooling: package managers (vcpkg, conan, build2, etc.)
# Tooling: static analysis
# Tooling: runtime analysis

# Tooling: shells (bash, zsh, )
# Tooling: zsh
RUN apt update && apt upgrade -y && \
    apt install -y zsh && \
    yes '' | sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)"
# see https://stackoverflow.com/questions/55987337/visual-studio-code-remote-containers-change-shell

# TODO: gdb, lldb

# C++ toolchain
# C++ toolchain: GCC

# simpler alternative: apt install with wildcard

#   apt install '^gcc\-[[:digit:]]{1,}$'
#   apt-cache search --quiet --names-only '^gcc\-[[:digit:]]{1,}$' | grep -oP 'gcc\-\K([[:digit:]]{1,})' | sort -n
#   apt-cache search --quiet --names-only '^gcc\-[[:digit:]]{1,}$' | grep -oP 'gcc\-\K([[:digit:]]{1,})' | sort -n | xargs -I {} echo "- {} ..."

# apt-cache search --quiet --names-only '^gcc\-[[:digit:]]{1,}$' \
#     | grep -oP 'gcc\-\K([[:digit:]]{1,})' \
#     | sort -n \
#     | xargs -I {} \
#     apt install -y gcc-{}


# GCC - version are either externaly provided, or all available are selected by default
ARG gcc_versions
RUN : {gcc_versions:=$(apt-cache search --quiet --names-only '^g\+\+\-[[:digit:]]{1,}$' | tr " " "\n" | grep -oP 'g\+\+\-\K([[:digit:]]{1,})' | sort -nu)}; \
    echo $gcc_versions | tr " " "\n" | xargs -I {} sh -c '\
        apt install -y --no-install-recommends  \
            g++-{} gcc-{}-multilib g++-{}-multilib                                                                \
            g++-{}-aarch64-linux-gnu g++-{}-arm-linux-gnueabihf                                                   \
            g++-{}-powerpc64-linux-gnu                                                                            \
            g++-{}-powerpc64le-linux-gnu  g++-{}-powerpc-linux-gnu                                                \
            binutils-aarch64-linux-gnu                                                                            \
            binutils-powerpc64-linux-gnu                                                                          \
        ;   \
        update-alternatives --install /usr/bin/gcc  gcc  /usr/bin/gcc-{} {} \
                            --slave   /usr/bin/g++  g++  /usr/bin/g++-{}    \
                            --slave   /usr/bin/gcov gcov /usr/bin/gcov-{};  \
    '


    apt install -y  build-essential \
                    libc6  libc6-dev libstdc++6 \
                    gcc-${GCC_VERSION} g++-${GCC_VERSION} \
                    gdb
# for each version, use {version} as priority ?
RUN update-alternatives --install /usr/bin/gcc  gcc  /usr/bin/gcc-${GCC_VERSION} 100    \
                        --slave   /usr/bin/g++  g++  /usr/bin/g++-${GCC_VERSION}        \
                        --slave   /usr/bin/gcov gcov /usr/bin/gcov-${GCC_VERSION}
# GCC: multi arch

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
# lld

# large/compiled libraries
# - https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz



# CMD ["zsh"]

# Cleanup

# QUESTION: ENV for CI -> gcc latest, clang latest ? or rely on alternative-updates ?

# ---
# docker compose:
# - allow mounting sources
# - allow remote: ssh + rsync connection
