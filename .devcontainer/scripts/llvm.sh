#!/bin/bash

set -eu

# =============================================================================================
# This file is part of https://github.com/GuillaumeDua/CppShelf,
# and will soon be part of https://hub.docker.com/repository/docker/gussd/cpp-toolchain/general
# License: see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
# =============================================================================================

this_script_name=$(basename "$0")

arg_versions='latest-stable'
arg_list=0
arg_silent=1
arg_alias=0
arg_minimalistic=0
arg_cleanup=0

internal_script_path='impl.sh'

help(){
    echo "Usage: ${this_script_name}" 1>&2
    echo "
    Boolean values: y|yes|1|true or n|no|0|false (case insensitive)

        [ -l | --list ]         : Only list available versions, expanding [versions].       Boolean -> default is [0]
        [ -v | --versions ]     : Versions to install.                                      String: all|latest|latest-stable|>=(number)|(space-separated-numbers...) -> default is [latest-stable]
            - [all]             : all versions availables                                       Ex: 'all'
            - [latest]          : only the latest        version available                      Ex: 'latest'
            - [latest-stable]   : only the latest-stable version available                      Ex: 'latest-stable'
            - [>=(number)]      : all versions greater or equal to <number>                     Ex: '>=42'
            - [numbers...]      : only listed versions.                                         Ex: '13 25 42' (space-separated)
        [ -s | --silent ]       : Run in silent mod.                                        Boolean -> default is [1]
        [ -a | --alias]         : Set bash/zsh-rc aliases.                                  Boolean -> default is [0]
        [ -m | --minimalistic]  : only clang/clang++, not tools.                            Boolean -> default is [0]
        [ -c | --cleanup]       : purge any (pre-)existing llvm/clang package installation: Boolean -> default is [0]
        [ -h | --help ]         : Display usage/help

    For instance, to only install the two latest versions available, use:
        sudo ./${this_script_name} --versions=\"\$(sudo ./${this_script_name} --list --versions='all' | tail -2)\"
        " 1>&2
    exit 0
}
clean(){
    if [ -f "${internal_script_path}" ]; then
        rm -rf "${internal_script_path}"
    fi
}
error(){
    echo -e "[${this_script_name}]: $@" >> /dev/stderr
    clean; exit 1
}
warning(){
    echo -e "[${this_script_name}]: $@" >> /dev/stderr
}
log(){
    if [[ "${arg_silent}" == 1 ]]; then
        return 0;
    fi
    echo -e "[${this_script_name}]: $@"
    return 0
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
fi

# --- options management ---

options_short=s:,v:,a:,m,c,l,h
options_long=silent:,versions:,alias:,minimalistic,cleanup,help,list
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
    -m | --minimalistic )
        arg_minimalistic=1
        shift;
        ;;
    -c | --cleanup )
        arg_cleanup=1
        shift;
        ;;
    -l | --list )
        arg_list=1
        shift;
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

arg_silent=$(to_boolean "${arg_silent}")
if [ "$arg_silent" == '' ] ; then
    exit 1;
fi

arg_list=$(to_boolean "${arg_list}")
if [ "$arg_list" == '' ] ; then
    exit 1;
fi

arg_minimalistic=$(to_boolean "${arg_minimalistic}")
if [ "$arg_minimalistic" == '' ] ; then
    exit 1;
fi

arg_cleanup=$(to_boolean "${arg_cleanup}")
if [ "$arg_cleanup" == '' ] ; then
    exit 1;
fi

log "arguments - versions:          [${arg_versions}]"
log "arguments - silent:            [${arg_silent}]"
log "arguments - alias:             [${arg_alias}]"
log "arguments - list:              [${arg_list}]"
log "arguments - minimalistic:      [${arg_minimalistic}]"
log "arguments - cleanup:           [${arg_cleanup}]"

# --- fetch llvm.sh ---
# or use:
#   sudo add-apt-repository "deb http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs) main"
#   wget https://apt.llvm.org/llvm-snapshot.gpg.key
#   sudo apt-key add llvm-snapshot.gpg.key

if [ -f "${internal_script_path}" ]; then
    echo -e "temporary file [${internal_script_path}] already exists" >> /dev/stderr # not using error to avoid deleting the file
    exit 1
fi

external_script_url='https://apt.llvm.org/llvm.sh'

# wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
# wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
# NO_PUBKEY 1A127079A92F09ED
# REFACTO: remove gpg key -> already added by ${external_script_url}
wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo gpg --dearmor --batch --yes -o /etc/apt/trusted.gpg.d/llvm-snapshot.gpg \
    && wget -qO ${internal_script_path} ${external_script_url} \
    && chmod +x "${internal_script_path}"
if [ $? != 0 ] || [ ! -f "${internal_script_path}" ]; then
    error "fetching [${external_script_url}] failed"
fi

# --- list versions ---

llvm_version_to_install_regex='^LLVM_VERSION_PATTERNS\[(\d+)\]=\"\-\K(\d+)'
list_to_install_llvm_versions(){
    grep -oP $llvm_version_to_install_regex ${internal_script_path} | uniq | sort -n
}
llvm_version_installed_regex='^clang-\K([0-9]{2})'
list_installed_llvm_versions(){
    dpkg -l | grep ^ii |  awk '{print $2}' | grep -oP $llvm_version_installed_regex | uniq | sort -n
}

# --- which versions ---
llvm_latest_stable=$(grep -oP '^CURRENT_LLVM_STABLE=\K(\d+)' ${internal_script_path})
all_llvm_versions_available=$(list_to_install_llvm_versions)

if [ "$arg_versions" = 'all' ]; then
    llvm_versions=$all_llvm_versions_available
elif [ "$arg_versions" = 'latest-stable' ]; then
    llvm_versions=$llvm_latest_stable
elif [ "$arg_versions" = 'latest' ]; then
    llvm_versions=$(echo ${all_llvm_versions_available} | tr " " "\n" | tail -1)
elif [[ "$arg_versions" =~  ^\>=[0-9]+$ ]]; then
    from_version=$(echo "${arg_versions}" | grep -oP '^>=\K([0-9])+$')
    log "using user-provided rule: >=[$from_version]"
    if [ -z "$from_version" ]; then
        error "invalid version='>=[0-9]+' value"
    fi
    llvm_versions=$(echo "$all_llvm_versions_available" | awk "\$1 >= ${from_version}")
elif [[ "$arg_versions" =~  ^[0-9]+( [0-9]+)*$ ]]; then
    log "using user-provided version(s) list: [${arg_versions}]"
    llvm_versions="${arg_versions}"
elif [ ! -z "$arg_versions" ]; then
    error "invalid value for argument version [${arg_versions}]"
fi

if [ -z "$llvm_versions" ]; then
    log "empty versions range, nothing to do"
    echo -e "$(list_installed_llvm_versions)" # result for the caller
    clean; exit 0
fi
if [[ ! $(echo -n $llvm_versions) =~  ^[0-9]+( [0-9]+)*$ ]]; then
    error "invalid versions range: [$llvm_versions]"
fi

## --- list mod ? ---
if [[ ${arg_list} == 1 ]]; then
    echo -e "${llvm_versions}"
    clean; exit 0
fi

log "LLVM version(s) to be installed: [${llvm_versions}]"

# --- installations ---

codename=$(lsb_release -cs)

# apt.llvm.org installs these with `all`. The clang-libcxx preset links against libc++, whose
# libc++.so is the linker script INPUT(libc++.so.1 -lc++abi -lunwind): hence the other two.
distro_packages(){
    local version="$1"
    local packages="clang-${version} libc++-${version}-dev libc++abi-${version}-dev libunwind-${version}-dev"
    if [[ ${arg_minimalistic} == 0 ]]; then
        packages="${packages} clang-format-${version} clang-tidy-${version} clang-tools-${version} clangd-${version} lldb-${version} llvm-${version}"
    fi
    echo "${packages}"
}

apt_candidate(){
    apt-cache policy "$1" 2>/dev/null | sed -n 's/^  Candidate: //p' | grep -vFx '(none)'
}

llvm_apt_org_serves(){
    wget -q --spider "https://apt.llvm.org/${codename}/dists/llvm-toolchain-${codename}-$1/Release"
}

# Buffering keeps a successful install quiet without losing the diagnostic from a failing one.
run_quietly(){
    local failure_message="$1"
    shift
    local output_file
    output_file=$(mktemp)
    if "$@" > "${output_file}" 2>&1; then
        rm -f "${output_file}"
        return 0
    fi
    cat "${output_file}" >&2
    rm -f "${output_file}"
    error "${failure_message}"
}

# apt.llvm.org carries only the most recent LLVM versions for each Ubuntu release, while the
# distribution archive keeps older ones: resolute (26.04) has no llvm-toolchain-resolute-18 yet packages clang-18.
# Resolving before --cleanup purges anything keeps an unavailable version from leaving the system with no clang.
mapfile -t llvm_versions_to_install < <(echo ${llvm_versions} | tr ' ' '\n')
declare -A install_source
for version in "${llvm_versions_to_install[@]}"; do
    if [ -n "$(apt_candidate "clang-${version}")" ]; then
        install_source["${version}"]="${codename} archive"
    elif llvm_apt_org_serves "${version}"; then
        install_source["${version}"]="${external_script_url}"
    else
        error "LLVM ${version} is packaged neither in the ${codename} archive nor at ${external_script_url}"
    fi
    log "LLVM ${version}: installing from [${install_source[${version}]}]"
done

# --- clean update-alternatives ---
sudo rm -rf /etc/alternatives/clang* /etc/alternatives/llvm-symbolizer /etc/alternatives/lldb
sudo rm -rf /var/lib/dpkg/alternatives/clang* /var/lib/dpkg/alternatives/llvm-symbolizer /var/lib/dpkg/alternatives/lldb

if [[ ${arg_cleanup} == 1 ]]; then
    sudo apt-get remove -y "llvm-*"
    sudo apt-get remove -y "lldb-*"
    sudo apt-get remove -y "clang-*"
    sudo apt-get remove -y "python3-lldb-*"
fi

for version in "${llvm_versions_to_install[@]}"; do

    if [ "${install_source[${version}]}" = "${external_script_url}" ]; then
        run_quietly "running [${external_script_url} ${version} all] failed" \
            "./${internal_script_path}" "${version}" all -n "${codename}"
    else
        # apt allows one lldb installation at a time, so this cannot use --no-remove.
        run_quietly "installing LLVM ${version} from the ${codename} archive failed" \
            apt-get install -qq -y --no-install-recommends $(distro_packages "${version}")
    fi

    # Latest stable always has the highest priority
    update_alternative_priority=$([[ "${version}" = "${llvm_latest_stable}" ]] && echo "100" || echo "${version}")

    if [[ ${arg_minimalistic} == 1 ]]; then
        update-alternatives --quiet                                                                                             \
            --install /usr/bin/clang clang /usr/bin/clang-${version} ${update_alternative_priority}                             \
            --slave /usr/bin/clang++                  clang++                   /usr/bin/clang++-${version}                     \
        || error "update-alternatives of [${version}] failed"
    else
        update-alternatives --quiet                                                                                             \
            --install /usr/bin/clang clang /usr/bin/clang-${version} ${update_alternative_priority}                             \
            --slave /usr/bin/clang++                  clang++                   /usr/bin/clang++-${version}                     \
            --slave /usr/bin/clang-format             clang-format              /usr/bin/clang-format-${version}                \
            --slave /usr/bin/clang-tidy               clang-tidy                /usr/bin/clang-tidy-${version}                  \
            --slave /usr/bin/clangd                   clangd                    /usr/bin/clangd-${version}                      \
            --slave /usr/bin/clang-check              clang-check               /usr/bin/clang-check-${version}                 \
            --slave /usr/bin/clang-query              clang-query               /usr/bin/clang-query-${version}                 \
            --slave /usr/bin/clang-apply-replacements clang-apply-replacements  /usr/bin/clang-apply-replacements-${version}    \
            --slave /usr/bin/sancov                   sancov                    /usr/bin/sancov-${version}                      \
            --slave /usr/bin/scan-build               scan-build                /usr/bin/scan-build-${version}                  \
            --slave /usr/bin/scan-view                scan-view                 /usr/bin/scan-view-${version}                   \
            --slave /usr/bin/llvm-symbolizer          llvm-symbolizer           /usr/bin/llvm-symbolizer-${version}             \
            --slave /usr/bin/lldb                     lldb                      /usr/bin/lldb-${version}                        \
        || error "update-alternatives of [${version}] failed"
    fi

done
clean;

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
