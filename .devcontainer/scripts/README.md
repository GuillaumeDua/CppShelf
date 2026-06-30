# Toolchain installation scripts

Standalone scripts to install `GCC` and `LLVM/Clang` toolchains, reusable on any Debian/Ubuntu-based system.  
Both require root privileges. Used by the [devcontainer](../Dockerfile) and the `WSL2` integrations.

---

## `gcc.sh`

```bash
sudo ./gcc.sh [options]
```

Installs one or more GCC versions from the `ubuntu-toolchain-r/test` PPA (added automatically if missing), sets up `update-alternatives` for `gcc`/`g++`/`gcov`, and best-effort installs the matching `-multilib` packages.

| Option             | Type    | Default         | Description                                                                                              |
| ------------------ | ------- | --------------- | -------------------------------------------------------------------------------------------------------- |
| `-v`, `--versions` | string  | `latest-stable` | `all` \| `latest` \| `latest-stable` \| `>=<number>` \| space-separated version numbers (e.g. `'13 14'`) |
| `-l`, `--list`     | boolean | `0`             | Only list the versions that `--versions` resolves to, without installing anything                        |
| `-s`, `--silent`   | boolean | `1`             | Suppress log output                                                                                      |
| `-a`, `--alias`    | boolean | `0`             | Append the resulting `gcc_versions` variable to `/etc/bash.bashrc` and `/etc/zsh/zshrc`                  |
| `-h`, `--help`     | —       | —               | Display usage                                                                                            |

Boolean values accept `y|yes|1|true` / `n|no|0|false` (case-insensitive).

**Example**: install the two latest available versions:

```bash
sudo ./gcc.sh --versions="$(sudo ./gcc.sh --list --versions='all' | tail -2)"
```

---

## `llvm.sh`

```bash
sudo ./llvm.sh [options]
```

Wraps the upstream [`apt.llvm.org/llvm.sh`](https://apt.llvm.org/llvm.sh) installer:

- fetches it (and the LLVM apt signing key) into a temporary `impl.sh`
- resolves the requested version(s)
- installs them
- then sets up `update-alternatives` for `clang`/`clang++` and (unless `--minimalistic`) the full toolchain (`clang-format`, `clang-tidy`, `clangd`, `lldb`, `scan-build`, ...)
- *The temporary `impl.sh` is removed before exit*

| Option                 | Type    | Default         | Description                                                                                              |
| ---------------------- | ------- | --------------- | -------------------------------------------------------------------------------------------------------- |
| `-v`, `--versions`     | string  | `latest-stable` | `all` \| `latest` \| `latest-stable` \| `>=<number>` \| space-separated version numbers (e.g. `'17 18'`) |
| `-l`, `--list`         | boolean | `0`             | Only list the versions that `--versions` resolves to, without installing anything                        |
| `-s`, `--silent`       | boolean | `1`             | Suppress log output                                                                                      |
| `-a`, `--alias`        | boolean | `0`             | Append the resulting `llvm_versions` variable to `/etc/bash.bashrc` and `/etc/zsh/zshrc`                 |
| `-m`, `--minimalistic` | boolean | `0`             | Only register `clang`/`clang++` alternatives, skip the extra tools                                       |
| `-c`, `--cleanup`      | boolean | `0`             | Purge any pre-existing `llvm-*`/`lldb-*`/`clang-*`/`python3-lldb-*` packages before installing           |
| `-h`, `--help`         | —       | —               | Display usage                                                                                            |

Boolean values accept `y|yes|1|true` / `n|no|0|false` (case-insensitive).

The latest stable version (per upstream's `CURRENT_LLVM_STABLE`) always gets `update-alternatives` priority `100`; other versions are prioritized by their version number.

Example: install the two latest available versions:

```bash
sudo ./llvm.sh --versions="$(sudo ./llvm.sh --list --versions='all' | tail -2)"
```
