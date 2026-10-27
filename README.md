# cinfo

**A tiny, fast Linux system information tool written in pure C.**

`cinfo` is a lightweight command-line utility for displaying useful information about your Linux system, including hardware, memory, processes, networking, uptime, and filesystem usage.

[**Source code**](https://github.com/unknownember/cinfo) · [**All releases**](https://github.com/unknownember/cinfo/releases) · [**Latest pre-release: v1.2.0-rc2**](https://github.com/unknownember/cinfo/releases/tag/v1.2.0-rc2)

> **Pre-release notice:** v1.2.0-rc2 is a testing release, not the stable v1.2.0 release.

## Features

- **System information** — OS, kernel, architecture, online CPU count, and username
- **Host information** — hostname and Linux distribution
- **CPU** — processor model, architecture, and online processor count
- **GPU** — graphics device vendor and device IDs, plus driver name when available
- **Memory** — total and available RAM
- **Load average** — 1-, 5-, and 15-minute system load averages
- **Uptime** — system uptime in a readable format
- **Processes** — current process count
- **Users** — logged-in terminal sessions
- **Networking** — IPv4 and IPv6 addresses for network interfaces
- **Disk usage** — total, used, and available space on the root filesystem
- **Combined report** — display all supported system information with one command

## Requirements

- Linux
- A C compiler, such as GCC or Clang
- GNU Make
- glibc

## Build from source

Clone the repository and compile:

```bash
git clone https://github.com/unknownember/cinfo.git
cd cinfo
make
```

Run the locally built executable:

```bash
./cinfo --all
```

## Install

Install the executable to `/usr/bin/cinfo`:

```bash
sudo make install
```

To remove the installed executable:

```bash
sudo rm /usr/bin/cinfo
```

To remove the locally built executable and build artifacts:

```bash
make clean
```

## Usage

Use `--help` to see the available options:

```bash
cinfo --help
```

Common commands:

```bash
cinfo --version
cinfo --cpu
cinfo --memory
cinfo --network
cinfo --all
```

### Command-line options

| Option | Description |
|---|---|
| `-h`, `--help` | Display help |
| `--version` | Display the cinfo version |
| `--sysinfo` | General system information |
| `--host` | Hostname and Linux distribution |
| `--cpu` | CPU model and processor information |
| `--gpu` | GPU device IDs and driver information |
| `--kernel` | Kernel version |
| `--memory` | Total and available RAM |
| `--load` | System load averages |
| `--uptime` | System uptime |
| `--processes` | Current process count |
| `--users` | Logged-in terminal sessions |
| `--network` | Network interface IP addresses |
| `--disk` | Root filesystem usage |
| `--all` | Display all supported system information |

## Arch Linux package

An Arch Linux `PKGBUILD` is included in this repository.

Build and install the package locally with:

```bash
makepkg -si
```

The package recipe downloads a versioned source archive from GitHub and verifies it using SHA-256.

## Latest pre-release: v1.2.0-rc2

[**Cinfo v1.2.0-rc2**](https://github.com/unknownember/cinfo/releases/tag/v1.2.0-rc2) includes improved terminal output, ANSI-colored section headings, configurable color modes, `NO_COLOR` support in automatic mode, and file fetching with glob expansion and combined file-size reporting.

The options below are available in **v1.2.0-rc2**; they are not yet part of the current `main` branch's command-line interface.

### `--fetch PATTERN`

Pass a file path or a quoted glob pattern. The fetcher lists matching regular files and prints their total size.

```bash
cinfo --fetch ./README.md
cinfo --fetch './src/*.c'
```

Quote glob patterns so the shell passes the pattern to `cinfo` for expansion. If no files match, `cinfo` reports the problem.

### `--color=MODE`

Configure ANSI color output with one of these modes:

| Mode | Behavior |
|---|---|
| `auto` | Enable colors when standard output is a terminal; respect a non-empty `NO_COLOR` environment variable |
| `always` | Force colors on |
| `never` | Disable colors |

Examples:

```bash
cinfo --color=auto --all
cinfo --color=always --cpu
cinfo --color=never --all
```

The default mode is `auto`.

As a release candidate, v1.2.0-rc2 is intended for testing and is not the stable v1.2.0 release. See the [releases page](https://github.com/unknownember/cinfo/releases) for available versions.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).

## Source

[github.com/unknownember/cinfo](https://github.com/unknownember/cinfo)
