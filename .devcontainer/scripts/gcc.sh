#!/bin/bash

this_script_name=$(basename "$0")

arg_silent=1
arg_versions='all'

help(){
    echo "Usage: ${this_script_name}"
    echo '
        [ -s | --silent ]   : (case insensitive) y|yes|1|true or n|no|0|false -> default is 1
        [ -v | --versions ] : (case insensitive) all, or a space-separated list of versions numbers to install -> default is all
        [ -h | --help ]'
    exit 0
}
error(){
    echo "[${this_script_name}]: " $@ >> /dev/stderr
    exit 1
}
log(){
    if [[ "${arg_silent}" == 1 ]]; then
        return 0;
    fi
    echo "[${this_script_name}]: " $@
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
is_all(){
    if [[ $# != 1 ]]; then
        error "$0: missing argument"
        exit 1
    fi
    case "$1" in
        [Aa][Ll][Ll]) echo 1;;
        *) echo 0;;
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

# options_count=$#
# if [ "$options_count" -eq 0 ]; then
#   help
# fi

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

log "silent:   [${arg_silent}]"
log "versions: [${arg_versions}]"

arg_silent=$(to_boolean $arg_silent)

# --- install versions ---

if [[ $(is_all $arg_versions) == 1 ]]; then
    gcc_versions=$(apt list --all-versions 2>/dev/null  | grep -oP '^gcc-\K([0-9]{2})' | sort -n | uniq)
elif [[ $arg_versions =~ "^\d+(\ \d+)*$" ]]; then
    log "user provided versions list will be installed: [${arg_versions}]"
    gcc_versions="${arg_versions}"
else
    error "invalid value for argument version [${arg_versions}]"
    exit 1
fi

log "GCC version to be installed: [" $gcc_versions "]"

# ARG gcc_versions
# RUN gcc_versions=${gcc_versions:=$(apt list --all-versions 2>/dev/null  | grep -oP '^gcc-\K([0-9]{2})' | sort -n | uniq)}; \
#     \
#     echo "[toolchain] Embedding gcc versions = [${gcc_versions}]";  \
#     echo gcc_versions=\'${gcc_versions}\' >> /etc/bash.bashrc;      \
#     # \'' fix coloration in vscode with docker extension ¯\_(ツ)_/¯
#     echo gcc_versions=\'${gcc_versions}\' >> /etc/zsh/zshrc;        \
#     # \'' fix coloration in vscode with docker extension ¯\_(ツ)_/¯
#     \
#     echo $gcc_versions | tr " " "\n" | xargs -I {} sh -c '          \
#         apt install -y --no-install-recommends                      \
#             gcc-{} g++-{}                                           \
#             gcc-{}-multilib g++-{}-multilib                         \
#         && update-alternatives                                      \
#             --install /usr/bin/gcc  gcc  /usr/bin/gcc-{} {}         \
#             --slave   /usr/bin/g++  g++  /usr/bin/g++-{}            \
#             --slave   /usr/bin/gcov gcov /usr/bin/gcov-{}           \
#     '

exit 0;