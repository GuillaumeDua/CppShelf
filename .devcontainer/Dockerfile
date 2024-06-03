# TODO: move to dedicated repo
# TODO: build image on weekly basis

ARG BASE_IMAGE=ubuntu:latest
FROM ${BASE_IMAGE} as cpp-toolchain-dev
ARG DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-c"] 

# Basics / installation prerequisites
RUN apt update -qqy && apt install -qqy --no-install-recommends \
        ca-certificates debian-keyring                      \
        gpg gpg-agent gnupg                                 \
        software-properties-common lsb-release apt-utils    \
        python3 pip                                         \
   && add-apt-repository ppa:ubuntu-toolchain-r/test        \
   && apt update -qqy && apt install -qqy --no-install-recommends \
        # remote
        wget ssh                                            \
        sudo tzdata curl libssl-dev                         \
        less tar zip unzip gzip                             \
        build-essential pkg-config                          \
        # build: CMake generators
        make ninja-build                                    \
        # build: cache
        ccache                                              \
        # versioning
        git subversion                                      \
        # dev: heavy libraries
        # https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz (+ detect, install latest instead of 1.84)
        # libboost-all-dev                                    \
        # tools: dev
        gdb                                                 \
        # tools: analysis (TODO: sonarlint)
        valgrind cppcheck iwyu                              \
        # tools: documentation
        doxygen graphviz                                    \
        # tools: editors
        emacs nano vim                                      \
        # tools: others
        docker-compose jq                                   \
        ripgrep                                             \
        # mlocate                                           \ # error: Unable to locate package mlocate :shrug:
    && rm -rf /var/lib/apt/lists/*

# Build: CMake (https://apt.kitware.com/)
#   quick-fix: Ubuntu-22.04-jammy instead of Ubuntu-24.04-noble -> kitware-archive.sh supports only Ubuntu releases up to Ubuntu-22.04-jammy
RUN internal_script_path='impl.sh';                                                                         \
    codename=$(value=$(lsb_release -cs); [[ "${value}" == "noble" ]] && value="jammy"; echo "${value}");    \
    wget -qO ${internal_script_path} https://apt.kitware.com/kitware-archive.sh                             \
    && chmod +x "${internal_script_path}"                                                                   \
    && ./${internal_script_path} --release ${codename}                                                      \
    && rm  -rf ${internal_script_path}                                                                      \
    && apt update -qqy && apt install -qqy --no-install-recommends                                          \
        cmake                                                                                               \
    && rm -rf /var/lib/apt/lists/*

# Build: Bazel (https://bazel.build/install/ubuntu)
ARG OPT_IN_INTEGRATE_BAZEL='no'
RUN if [[ "${OPT_IN_INTEGRATE_BAZEL}" = "y" ]]; then                    \
        apt update -qqy && apt install -qqy --no-install-recommends     \
            apt-transport-https curl gnupg                              \
        && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg \
        && mv bazel-archive-keyring.gpg /usr/share/keyrings             \
        && echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list \
        && apt update -qqy && apt install -qqy --no-install-recommends  \ 
            bazel                                                       \
        && rm -rf /var/lib/apt/lists/*                                  \
        ;                                                               \
    fi

ENV TOOLCHAIN_TMP_DIR=/tmp/install_toolchain/
# Dependency managers
ENV VCPKG_URL       https://github.com/microsoft/vcpkg/archive/master.tar.gz
RUN \
    # vcpkg
    wget -qO vcpkg.tar.gz ${VCPKG_URL}                          \
    && mkdir /opt/vcpkg                                         \
    && tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg   \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics            \
    && ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg
RUN \
    # conan
    #   work-around for pip "error: externally-managed-environment"
    apt update -qqy && apt install -qqy --no-install-recommends \
        pipx                                                    \
    # && pipx --quiet install conan                             \ # this feature is not released yet
    && (pipx install conan > /dev/null 2>&1)                    \
    && rm -rf /var/lib/apt/lists/*

# TODO
# - Tooling: static analysis
# - Tooling: runtime analysis

# C++ toolchain
RUN apt update -qqy && apt install -qqy --no-install-recommends \
        libc6 libc6-dev libstdc++6 libc6-arm64-cross            \
        binutils-aarch64-linux-gnu binutils-powerpc64-linux-gnu

# C++ toolchain: GNU/GCC
COPY ./scripts/gcc.sh ${TOOLCHAIN_TMP_DIR}/scripts/gcc.sh
WORKDIR ${TOOLCHAIN_TMP_DIR}
ARG GCC_VERSIONS='>=11'
RUN script_path=${TOOLCHAIN_TMP_DIR}/scripts/gcc.sh;                        \
    echo -e "[C++ toolchain] Installing GCC_VERSIONS=[$GCC_VERSIONS] ..." ; \
    chmod +x ${script_path}                                                 \
    && ${script_path} --silent=yes --alias=yes --versions="$GCC_VERSIONS"

# C++ toolchain: LLVM/Clang (https://apt.llvm.org/)
COPY ./scripts/llvm.sh ${TOOLCHAIN_TMP_DIR}/scripts/llvm.sh
WORKDIR ${TOOLCHAIN_TMP_DIR}
ARG LLVM_VERSIONS='>=14'
RUN script_path=${TOOLCHAIN_TMP_DIR}/scripts/llvm.sh;                           \
    echo -e "[C++ toolchain] Installing LLVM_VERSIONS=[$LLVM_VERSIONS] ..." ;   \
    chmod +x ${script_path}                                                     \
    && ${script_path} --silent=yes --alias=yes --versions="$LLVM_VERSIONS"

# Build: Build2-0.16.0 (depends on a compiler)
ARG OPT_IN_INTEGRATE_BUILD2='no'
RUN if [[ "${OPT_IN_INTEGRATE_BUILD2}" = "y" ]]; then                               \
        mkdir -p /tmp/build2-build && cd /tmp/build2-build                          \
        && curl -sSfO https://download.build2.org/0.16.0/build2-install-0.16.0.sh   \
        && echo '4d2babfaff6d0412e0ab61a3d3c68a87de54c0779254d62f4569b44a56c6ea08 *build2-install-0.16.0.sh' | shasum -a 256 -c \
        && sh build2-install-0.16.0.sh                                              \
            --yes                                                                   \
            --cxx clang++                                                           \
            --sudo false                                                            \
            --jobs $(nproc)                                                         \
        ;                                                                           \
    fi

# Tooling: shells - bash, zsh
# TODO: for all users
RUN apt update -qqy && apt install -qqy --no-install-recommends \
        bash zsh \
    # && yes '' | sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)" \
    # ZSH: Default powerline10k theme, no plugins installed
    && yes | sh -c "$(wget -O- https://github.com/deluan/zsh-in-docker/releases/download/v1.1.5/zsh-in-docker.sh)" \
    && rm -rf /var/lib/apt/lists/*
# see https://stackoverflow.com/questions/55987337/visual-studio-code-remote-containers-change-shell

# Cleanup
RUN apt clean \
    && rm -rf ${TOOLCHAIN_TMP_DIR} \
    && rm -rf /var/lib/apt/lists/*
