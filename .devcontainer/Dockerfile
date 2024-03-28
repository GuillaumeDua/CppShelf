# TODO: move to dedicated repo
# TODO: build image on weekly basis

FROM ubuntu:latest as build-toolchain

ENV DEBIAN_FRONTEND noninteractive

# Basics / installation prerequisites
RUN apt update && apt install -y --no-install-recommends \
        gpg-agent debian-keyring \
        software-properties-common lsb-release apt-utils \
        python3 pip \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get update -y \
    && apt install -y \
        # remote
        wget ssh \
        sudo tzdata curl libssl-dev ca-certificates \
        less tar zip unzip gzip \
        build-essential pkg-config \
        # build
        make ninja-build \
        # versioning
        git subversion \
        # libraries
        libboost-all-dev \
        # tools
        doxygen graphviz \
        emacs nano vim \
    && rm -rf /var/lib/apt/lists/*

# CMake
# RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
#     && apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" \
#     && apt install kitware-archive-keyring \
#     && rm /etc/apt/trusted.gpg.d/kitware.gpg \
#     && apt-get update -y \
#     && apt install -y cmake

# package managers
RUN \
    # vcpkg
    wget -qO vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz \
    && mkdir /opt/vcpkg \
    && tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh \
    # conan
    && ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg \
    && pip install conan

# Tooling: package managers (vcpkg, conan, build2, etc.)
# Tooling: static analysis
# Tooling: runtime analysis

# Tooling: shells (bash, zsh, )
# Tooling: zsh
RUN apt update -y && apt upgrade -y \
    && apt install -y \
        bash zsh \
    && yes '' | sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)" \
    && rm -rf /var/lib/apt/lists/*
# see https://stackoverflow.com/questions/55987337/visual-studio-code-remote-containers-change-shell

# TODO: gdb, lldb

# C++ toolchain

RUN apt update -y && apt-get install -y --no-install-recommends \
    libc6 libc6-dev libstdc++6 libc6-arm64-cross

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

# LLVM
# RUN apt install -y wget bash \
#     && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - \
#     && wget https://apt.llvm.org/llvm.sh \  
#     && chmod +x llvm.sh \
#     && (yes '' | ./llvm.sh $CLANG_VERSION all) \
#     && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$CLANG_VERSION 100      \
#         --slave /usr/bin/clang++         clang++         /usr/bin/clang++-$CLANG_VERSION         \
#         --slave /usr/bin/clang-format    clang-format    /usr/bin/clang-format-$CLANG_VERSION    \
#         --slave /usr/bin/clang-tidy      clang-tidy      /usr/bin/clang-tidy-$CLANG_VERSION      \
#         --slave /usr/bin/clangd          clangd          /usr/bin/clangd-$CLANG_VERSION          \
#         --slave /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-$CLANG_VERSION

# lld, lldb

# large/compiled libraries
# - https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz

# Remote
RUN apt-get update -y \
    && apt install -y \
        openssh-server wget rsync ssh \
    && useradd -ms /bin/bash vscodeuser \
    && echo 'vscodeuser:password' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config \
    && echo 'AllowUsers vscodeuser' >> /etc/ssh/sshd_config
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]

# CMD ["zsh"]
# ENTRYPOINT ["zsh"]

# Cleanup

# QUESTION: ENV for CI -> gcc latest, clang latest ? or rely on alternative-updates ?

# ---
# docker compose:
# - allow mounting sources
# - allow remote: ssh + rsync connection
