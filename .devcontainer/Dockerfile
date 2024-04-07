# TODO: move to dedicated repo
# TODO: build image on weekly basis

FROM ubuntu:latest as cpp-build-toolchain

ENV DEBIAN_FRONTEND noninteractive

# Basics / installation prerequisites
RUN apt update && apt install -y --no-install-recommends    \
        ca-certificates gpg gpg-agent debian-keyring        \
        software-properties-common lsb-release apt-utils    \
        python3 pip                                         \
    && add-apt-repository ppa:ubuntu-toolchain-r/test       \
    && apt-get update -y                                    \
    && apt install -y                                       \
        # remote
        wget ssh                                            \
        sudo tzdata curl libssl-dev                         \
        less tar zip unzip gzip                             \
        build-essential pkg-config                          \
        # build: CMake generators
        make ninja-build                                    \
        # versioning
        git subversion                                      \
        # dev: heavy libraries
        # https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
        libboost-all-dev                                    \
        # tools
        doxygen graphviz                                    \
        # tools: editors
        emacs nano vim                                      \
    && rm -rf /var/lib/apt/lists/*

# -------------
# Build systems
# -------------

# Build: CMake (https://apt.kitware.com/)
# RUN apt-get update -y  \
#     && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
#     && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null \
#     && apt-get update -y  \
#     && rm /usr/share/keyrings/kitware-archive-keyring.gpg \
#     && apt-get install -y kitware-archive-keyring \
#     && apt install -y cmake

# Build: CMake (https://apt.kitware.com/)
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
    && apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" \
    && apt-get update -y                        \
    && apt install kitware-archive-keyring      \
    && rm /etc/apt/trusted.gpg.d/kitware.gpg    \
    # quick-fix ¯\_(ツ)_/¯
    && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1A127079A92F09ED \
    && apt-get update -y                        \
    && apt install -y cmake                     \
    && rm -rf /var/lib/apt/lists/*

# Build: Bazel (https://bazel.build/install/ubuntu)
RUN apt install apt-transport-https curl gnupg -y           \
    && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg \
    && mv bazel-archive-keyring.gpg /usr/share/keyrings     \
    && echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list \
    && apt update -y && apt install -y bazel

# Dependency managers
RUN \
    # vcpkg
    wget -qO vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz \
    && mkdir /opt/vcpkg \
    && tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh \
    # conan
    && ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg \
    && pip install conan

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
    libc6 libc6-dev libstdc++6 libc6-arm64-cross \
    binutils-aarch64-linux-gnu binutils-powerpc64-linux-gnu

# C++ toolchain: GCC
# GCC - version are either externaly provided, or all available are selected by default
# - simpler alternative: apt install with wildcard ¯\_(ツ)_/¯
ARG gcc_versions
RUN gcc_versions=${gcc_versions:=$(apt-cache search --quiet --names-only '^g\+\+\-[[:digit:]]{1,}$' | tr " " "\n" | grep -oP 'g\+\+\-\K([[:digit:]]{1,})' | sort -nu)}; \
    \
    echo "[toolchain] Embedding gcc versions = [${gcc_versions}]";  \
    echo gcc_versions=\'${gcc_versions}\' >> /etc/bash.bashrc;      \
    echo gcc_versions=\'${gcc_versions}\' >> /etc/zsh/zshrc;        \
    \
    echo $gcc_versions | tr " " "\n" | xargs -I {} sh -c '          \
        apt install -y --no-install-recommends                      \
            gcc-{} g++-{}                                           \
            gcc-{}-multilib g++-{}-multilib                         \
        && update-alternatives                                      \
            --install /usr/bin/gcc  gcc  /usr/bin/gcc-{} {}         \
            --slave   /usr/bin/g++  g++  /usr/bin/g++-{}            \
            --slave   /usr/bin/gcov gcov /usr/bin/gcov-{}           \
    '

# Not available for g++-13
# g++-{}-aarch64-linux-gnu g++-{}-arm-linux-gnueabihf         \
# g++-{}-powerpc64-linux-gnu g++-{}-powerpc64le-linux-gnu  g++-{}-powerpc-linux-gnu      \

# # C++ toolchain: LLVM
# ARG llvm_versions
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

# Remote
RUN apt-get update -y \
    && apt install -y \
        openssh-server wget rsync ssh \
    && useradd -ms /bin/bash vscodeuser \
    && echo 'vscodeuser:password' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config \
    && echo 'AllowUsers vscodeuser' >> /etc/ssh/sshd_config \
    && mkdir -p mkdir /var/run/sshd \
    && service ssh stop
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
