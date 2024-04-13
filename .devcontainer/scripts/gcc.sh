#!/bin/bash

this_script_name=$(basename "$0")

help()
{
    echo "Usage: ${this_script_name}
        [ -s | --silent ]
        [ -v | --versions ]
        [ -h | --help ]"
    exit 1
}

# --- options management ---

options_short=s:,v:,h
options_long=silent:,versions:,help
getopt_result=$(getopt -a -n ${this_script_name} --options ${options_short} --longoptions ${options_long} -- "$@")

# options_count=$#
# if [ "$options_count" -eq 0 ]; then
#   help
# fi

arg_silent=1
arg_versions='all'

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
      echo "${this_script_name}: Unexpected option: [$1]"
      help
      ;;
  esac
done

echo "silent:   [${arg_silent}]";
echo "versions: [${arg_versions}]";

exit 0;

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