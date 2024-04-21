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

internal_script_path='impl.sh'
if [ -f "${internal_script_path}" ]; then
    rm "${internal_script_path}"
fi

## key
wget -q -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - \
    && wget -q -O impl.sh https://apt.llvm.org/llvm.sh \
    && chmod +x "${internal_script_path}"
if [ $? != 0 ] || [ ! -f "${internal_script_path}" ]; then
    error 'fetching [https://apt.llvm.org/llvm.sh] failed'
    exit 1
fi

# --- list versions ---

llvm_version_regex='LLVM_VERSION_PATTERNS\[(\d+)\]=\"\-\K(\d+)'
list_installed_gcc_versions(){
    cat ${internal_script_path} | grep -oP $gcc_version_regex | uniq | sort -n
}

echo -e $(list_installed_gcc_versions)

# --- install versions ---