#!/bin/bash

this_script_name=$(basename "$0")

arg_silent=1
arg_versions='all'

help(){
    echo "Usage: ${this_script_name}" 1>&2
    echo '
        [ -s | --silent ]   : y|yes|1|true or n|no|0|false (case insensitive) -> default is [1]
        [ -v | --versions ] : all|latest|(space-separated list of versions numbers to install) -> default is [all]
        [ -h | --help ]' 1>&2
    exit 0
}
error(){
    echo "[${this_script_name}]: $@" >> /dev/stderr
    exit 1
}
log(){
    if [[ "${arg_silent}" == 1 ]]; then
        return 0;
    fi
    echo "[${this_script_name}]: $@"
}
to_boolean(){
    if [[ $# != 1 ]]; then
        error "$0: missing argument"
        exit 1
    fi
    case "$1" in
        [Yy]|[Yy][Ee][Ss]|1|[Tt][Rr][Uu][Ee]) echo 1;;
        [Nn]|[Nn][Oo]|0|[Ff][Aa][Ll][Ss][Ee]) echo 0;;
        *) error "to_boolean: invalid conversion from [$1] to boolean" && exit 1;;
    esac
}

# --- precondition: sudoer ---

if [ "$EUID" -ne 0 ]; then
  error "Requires root privileges"
  exit 1
fi

# --- options management ---

options_short=s:,v:,h
options_long=silent:,versions:,help
getopt_result=$(getopt -a -n ${this_script_name} --options ${options_short} --longoptions ${options_long} -- "$@")

eval set -- "$getopt_result"

while :
do
  case "$1" in
    -s | --silent )
      arg_silent="$2"
      shift 2
      ;;
    -v | --versions )
      arg_versions="$2"
      shift 2
      ;;
    -h | --help)
      help
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

log "arguments - silent:   [${arg_silent}]"
log "arguments - versions: [${arg_versions}]"

arg_silent=$(to_boolean $arg_silent)

# --- install versions ---

all_gcc_versions_available=$(apt list --all-versions 2>/dev/null  | grep -oP '^gcc-\K([0-9]{2})' | sort -n | uniq)
if [ "$arg_versions" = 'all' ]; then
    gcc_versions=$all_gcc_versions_available
elif [ "$arg_versions" = 'latest' ]; then
    gcc_versions=$(echo ${all_gcc_versions_available} | tr " " "\n" | tail -1)
elif [[ "$arg_versions" =~  ^[0-9]+( [0-9]+)*$ ]]; then
    log "using user provided version(s) list: [${arg_versions}]"
    gcc_versions="${arg_versions}"
else
    error "invalid value for argument version [${arg_versions}]"
    exit 1
fi

log "GCC version to be installed: [" $gcc_versions "]"

# --- installations ---
mapfile -t gcc_versions_to_install < <(echo $gcc_versions | tr " " "\n")

for version in "${gcc_versions_to_install[@]}"; do
    log "installing ${version} ..."

    apt install -y --no-install-recommends                                      \
            gcc-${version} g++-${version}                                       \
            gcc-${version}-multilib g++-${version}-multilib                     \
        || error "installation of [${version}] failed"
    update-alternatives                                                         \
            --install /usr/bin/gcc  gcc  /usr/bin/gcc-${version} ${version}     \
            --slave   /usr/bin/g++  g++  /usr/bin/g++-${version}                \
            --slave   /usr/bin/gcov gcov /usr/bin/gcov-${version}               \
        || error "update-alternatives of [${version}] failed"

done

exit 0;