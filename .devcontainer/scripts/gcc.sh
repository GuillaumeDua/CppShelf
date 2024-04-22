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

# --- list versions ---

gcc_version_regex='^gcc-\K([0-9]{2})'
list_installed_gcc_versions(){
    dpkg -l | grep ^ii |  awk '{print $2}' | grep -oP $gcc_version_regex | uniq | sort -n
    # apt list --installed | grep -oP $gcc_version_regex | uniq | sort -n | xargs
}

# --- install versions ---

all_gcc_versions_available=$(apt list --all-versions 2>/dev/null  | grep -oP $gcc_version_regex | sort -n | uniq)
if [ "$arg_versions" = 'all' ]; then
    gcc_versions=$all_gcc_versions_available
elif [ "$arg_versions" = 'latest' ]; then
    gcc_versions=$(echo ${all_gcc_versions_available} | tr " " "\n" | tail -1)
elif [[ "$arg_versions" =~  ^from=[0-9]+$ ]]; then
    from_version=$(echo "${arg_versions}" | grep -oP '^from=\K([0-9])+$')
    log "using user-provided rule: >= $from_version"
    if [ -z "$from_version" ]; then
        error "invalid version='from=\"some_digit\"' value"
        exit 1
    fi
    gcc_versions=$(echo "$all_gcc_versions_available" | awk "\$1 >= ${arg_versions}")
elif [[ "$arg_versions" =~  ^[0-9]+( [0-9]+)*$ ]]; then
    log "using user-provided version(s) list: [${arg_versions}]"
    gcc_versions="${arg_versions}"
elif [ ! -z "$gcc_versions" ]; then
    error "invalid value for argument version [${arg_versions}]"
    exit 1
fi

if [ -z "$gcc_versions" ]; then
    log "invalid or empty versions range, nothing to do"
    echo -e "$(list_installed_gcc_versions)" # result for the caller
    exit 0
fi
if [[ ! $(echo -n $gcc_versions) =~  ^[0-9]+( [0-9]+)*$ ]]; then
    error "invalid versions range: [$gcc_versions]"
    exit 1
fi

## --- list mod ? ---
if [[ ${arg_list} == 1 ]]; then
    echo -e "${gcc_versions}"
    exit 0
fi

log "GCC version(s) to be installed: [${gcc_versions}]"

# --- installations ---
mapfile -t gcc_versions_to_install < <(echo -n "$gcc_versions")

for version in "${gcc_versions_to_install[@]}"; do
    log "installing ${version} ..."

    apt install -qq -y --no-install-recommends                                  \
            gcc-${version}          g++-${version}                              \
            gcc-${version}-multilib g++-${version}-multilib                     \
        || error "installation of [${version}] failed"
    update-alternatives                                                         \
            --install /usr/bin/gcc  gcc  /usr/bin/gcc-${version} ${version}     \
            --slave   /usr/bin/g++  g++  /usr/bin/g++-${version}                \
            --slave   /usr/bin/gcov gcov /usr/bin/gcov-${version}               \
        || error "update-alternatives of [${version}] failed"

done

# --- summary ---
gcc_versions=$(list_installed_gcc_versions)
log "GCC version now detected: [${gcc_versions}]"
echo -e "${gcc_versions}" # result for the caller

# --- Create aliases ---
arg_alias=$(to_boolean "${arg_alias}")
if [ "$arg_alias" == '' ] ; then
    exit 1;
fi

if [[ "${arg_alias}" == 1 ]]; then
    log "alias: adding aliases for [bash zsh]"
    [[ -f '/etc/bash.bashrc' ]] && echo gcc_versions=\'${gcc_versions}\' >> /etc/bash.bashrc;
    [[ -f '/etc/zsh/zshrc' ]]   && echo gcc_versions=\'${gcc_versions}\' >> /etc/zsh/zshrc;
fi

exit 0;