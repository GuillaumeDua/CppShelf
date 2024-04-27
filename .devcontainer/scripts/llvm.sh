#!/bin/bash

# =============================================================================================
# This file is part of https://github.com/GuillaumeDua/CppShelf,
# and will soon be part of https://hub.docker.com/repository/docker/gussd/cpp-toolchain/general
# License: see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
# =============================================================================================

this_script_name=$(basename "$0")

arg_versions='all'
arg_list=0
arg_silent=1
arg_alias=0

help(){
    echo "Usage: ${this_script_name}" 1>&2
    echo "
    Boolean values: y|yes|1|true or n|no|0|false (case insensitive)

        [ -l | --list ]     : Only list available versions. Boolean -> default is [0]
        [ -v | --versions ] : Versions to install.          String: all|latest|(space-separated of numbers) -> default is [all]
        [ -s | --silent ]   : Run in silent mod.            Boolean -> default is [1]
        [ -a | --alias]     : Set bash/zsh-rc aliases.      Boolean -> default is [0]
        [ -h | --help ]     : Display usage/help

    For instance, to only install the two latest versions, use:
        sudo ./${this_script_name} --versions=\"\$(sudo ./${this_script_name} -l | tail -2)\"
        " 1>&2
    exit 0
}
error(){
    echo -e "[${this_script_name}]: $@" >> /dev/stderr
    exit 1
}
log(){
    if [[ "${arg_silent}" == 1 ]]; then
        return 0;
    fi
    echo -e "[${this_script_name}]: $@"
}
to_boolean(){
    if [[ $# != 1 ]]; then
        error "$0: missing argument"
        exit 1
    fi
    case "$1" in
        [Yy]|[Yy][Ee][Ss]|1|[Tt][Rr][Uu][Ee]) echo 1;;
        [Nn]|[Nn][Oo]|0|[Ff][Aa][Ll][Ss][Ee]) echo 0;;
        *)
            error "to_boolean: invalid conversion from [$1] to boolean"
            exit 1
            ;;
    esac
}

# --- precondition: sudoer ---

if [ "$EUID" -ne 0 ]; then
  error "Requires root privileges"
  exit 1
fi

# --- options management ---

options_short=s:,v:,a:,l,h
options_long=silent:,versions:,alias:,help,list
getopt_result=$(getopt -a -n ${this_script_name} --options ${options_short} --longoptions ${options_long} -- "$@")

eval set -- "$getopt_result"

while :
do
  case "$1" in
    -s | --silent )
      arg_silent="$2"
      shift 2
      ;;
    -a | --alias )
      arg_alias="$2"
      shift 2
      ;;
    -v | --versions )
      arg_versions=$(echo $2 | tr -d '\n' | tr '\n' ' ')
      shift 2
      ;;
    -l | --list )
      arg_list=1
      arg_silent=1
      arg_versions='all'
      shift;
      break
      ;;
    -h | --help)
      help
      exit 0
      shift
      ;;
    --)
      shift;
      break
      ;;
    *)
      echo "${this_script_name}: Unexpected option: [$1]" >> /dev/stderr
      help
      ;;
  esac
done

log "arguments - versions: [${arg_versions}]"
log "arguments - silent:   [${arg_silent}]"
log "arguments - alias:    [${arg_alias}]"

arg_silent=$(to_boolean "${arg_silent}")
if [ "$arg_silent" == '' ] ; then
    exit 1;
fi

arg_list=$(to_boolean "${arg_list}")
if [ "$arg_list" == '' ] ; then
    exit 1;
fi

# --- fetch llvm.sh ---
# or use:
#   sudo add-apt-repository 'deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic main'
#   wget https://apt.llvm.org/llvm-snapshot.gpg.key
#   sudo apt-key add llvm-snapshot.gpg.key

internal_script_path='impl.sh'
if [ -f "${internal_script_path}" ]; then
    rm "${internal_script_path}"
fi

# NO_PUBKEY 1A127079A92F09ED
wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo gpg --dearmor --batch --yes -o /etc/apt/trusted.gpg.d/llvm-snapshot.gpg \
    && wget -qO impl.sh https://apt.llvm.org/llvm.sh \
    && chmod +x "${internal_script_path}"
if [ $? != 0 ] || [ ! -f "${internal_script_path}" ]; then
    error 'fetching [https://apt.llvm.org/llvm.sh] failed'
    exit 1
fi

# --- list versions ---

llvm_version_to_install_regex='LLVM_VERSION_PATTERNS\[(\d+)\]=\"\-\K(\d+)'
list_to_install_llvm_versions(){
    cat ${internal_script_path} | grep -oP $llvm_version_to_install_regex | uniq | sort -n
}
llvm_version_installed_regex='^clang-\K([0-9]{2})'
list_installed_llvm_versions(){
    dpkg -l | grep ^ii |  awk '{print $2}' | grep -oP $llvm_version_installed_regex | uniq | sort -n
}

# --- which versions ---

all_llvm_versions_available=$(list_to_install_llvm_versions)

if [ "$arg_versions" = 'all' ]; then
    llvm_versions=$all_llvm_versions_available
elif [ "$arg_versions" = 'latest' ]; then
    llvm_versions=$(echo ${all_llvm_versions_available} | tr " " "\n" | tail -1)
elif [[ "$arg_versions" =~  ^\>=[0-9]+$ ]]; then
    from_version=$(echo "${arg_versions}" | grep -oP '^>=\K([0-9])+$')
    log "using user-provided rule: >=[$from_version]"
    if [ -z "$from_version" ]; then
        error "invalid version='>=[0-9]+' value"
        exit 1
    fi
    llvm_versions=$(echo "$all_llvm_versions_available" | awk "\$1 >= ${from_version}")
elif [[ "$arg_versions" =~  ^[0-9]+( [0-9]+)*$ ]]; then
    log "using user-provided version(s) list: [${arg_versions}]"
    llvm_versions="${arg_versions}"
elif [ ! -z "$arg_versions" ]; then
    error "invalid value for argument version [${arg_versions}]"
    exit 1
fi

if [ -z "$llvm_versions" ]; then
    log "empty versions range, nothing to do"
    echo -e "$(list_installed_llvm_versions)" # result for the caller
    exit 0
fi
if [[ ! $(echo -n $llvm_versions) =~  ^[0-9]+( [0-9]+)*$ ]]; then
    error "invalid versions range: [$llvm_versions]"
    exit 1
fi

## --- list mod ? ---
if [[ ${arg_list} == 1 ]]; then
    echo -e "${llvm_versions}"
    exit 0
fi

log "LLVM version(s) to be installed: [${llvm_versions}]"

# --- clean update-alternatives ---
sudo rm -rf /etc/alternatives/clang* /etc/alternatives/llvm-symbolizer /etc/alternatives/lldb
sudo rm -rf /var/lib/dpkg/alternatives/clang* /var/lib/dpkg/alternatives/llvm-symbolizer /var/lib/dpkg/alternatives/lldb
# --- installations ---
yes '' | ./${internal_script_path} $llvm_versions all # TODO: quiet
sudo rm -rf ${internal_script_path}

mapfile -t llvm_versions_to_install < <(echo -n "$llvm_versions")

for version in "${llvm_versions_to_install[@]}"; do

    apt install -qq -y --no-install-recommends \
      clang-format-${version} \
      clang-tidy-${version}   \
      lldb-${version}
    # clang and clang-tools
    update-alternatives                                                                                                   \
        --install /usr/bin/clang clang /usr/bin/clang-${version} ${version}                                               \
        --slave /usr/bin/clang++                  clang++                   /usr/bin/clang++-${version}                   \
        --slave /usr/bin/clang-format             clang-format              /usr/bin/clang-format-${version}              \
        --slave /usr/bin/clang-tidy               clang-tidy                /usr/bin/clang-tidy-${version}                \
        --slave /usr/bin/clangd                   clangd                    /usr/bin/clangd-${version}                    \
        --slave /usr/bin/clang-check              clang-check               /usr/bin/clang-check-${version}               \
        --slave /usr/bin/clang-query              clang-query               /usr/bin/clang-query-${version}               \
        --slave /usr/bin/clang-apply-replacements clang-apply-replacements  /usr/bin/clang-apply-replacements-${version}  \
        --slave /usr/bin/sancov                   sancov                    /usr/bin/sancov-${version}                    \
        --slave /usr/bin/scan-build               scan-build                /usr/bin/scan-build-${version}                \
        --slave /usr/bin/scan-view                scan-view                 /usr/bin/scan-view-${version}                 \
        --slave /usr/bin/llvm-symbolizer          llvm-symbolizer           /usr/bin/llvm-symbolizer-${version}           \
        --slave /usr/bin/lldb                     lldb                      /usr/bin/lldb-${version}

done

# --- summary ---
llvm_versions=$(list_installed_llvm_versions)
log "LLVM versions now detected: [$(echo -e $(list_installed_llvm_versions))]" 
echo -e "${llvm_versions}" # result for the caller

# --- Create aliases ---
arg_alias=$(to_boolean "${arg_alias}")
if [ "$arg_alias" == '' ] ; then
    exit 1;
fi

if [[ "${arg_alias}" == 1 ]]; then
    log "alias: adding aliases for [bash zsh]"
    [[ -f '/etc/bash.bashrc' ]] && echo llvm_versions=\'${llvm_versions}\' >> /etc/bash.bashrc;
    [[ -f '/etc/zsh/zshrc' ]]   && echo llvm_versions=\'${llvm_versions}\' >> /etc/zsh/zshrc;
fi

exit 0;

# Legacy inline integration
#
# ARG llvm_versions=all
# RUN apt install -y wget bash \
#     && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - \
#     && wget https://apt.llvm.org/llvm.sh \  
#     && chmod +x llvm.sh \
#     && llvm_versions=${llvm_versions:=$(cat llvm.sh | grep -oP 'LLVM_VERSION_PATTERNS\[(\d+)\]=\"\-\K(\d+)' | sort -n)} \
#     \
#     echo "[toolchain] Embedding llvm versions = [${llvm_versions}]";    \
#     echo llvm_versions=\'${llvm_versions}\' >> /etc/bash.bashrc;        \
#     # \'' fix coloration in vscode with docker extension ¯\_(ツ)_/¯
#     echo llvm_versions=\'${llvm_versions}\' >> /etc/zsh/zshrc;          \
#     # \'' fix coloration in vscode with docker extension ¯\_(ツ)_/¯
#     \
#     && (yes '' | ./llvm.sh $llvm_versions) \
#     && echo $llvm_versions | tr " " "\n" | xargs -I {} sh -c '          \
#         update-alternatives                                                                 \
#             --install /usr/bin/clang clang /usr/bin/clang-{} {}                             \
#             --slave /usr/bin/clang++         clang++         /usr/bin/clang++-{}            \
#             --slave /usr/bin/clang-format    clang-format    /usr/bin/clang-format-{}       \
#             --slave /usr/bin/clang-tidy      clang-tidy      /usr/bin/clang-tidy-{}         \
#             --slave /usr/bin/clangd          clangd          /usr/bin/clangd-{}             \
#             --slave /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-{}    \
#             --slave /usr/bin/lldb            lldb            /usr/bin/lldb-{}               \
#     '
