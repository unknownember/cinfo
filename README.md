# cinfo

**A small, fast Linux system information utility written primarily in C.**

`cinfo` displays useful information about a Linux system, including CPU and GPU details, memory, kernel, uptime, processes, networking, and filesystem usage.

[**Source code**](https://github.com/unknownember/cinfo) · [**All releases**](https://github.com/unknownember/cinfo/releases) · [**Latest pre-release: v1.2.0-rc2**](https://github.com/unknownember/cinfo/releases/tag/v1.2.0-rc2)

> **Pre-release notice:** v1.2.0-rc2 is intended for testing. It is not the stable v1.2.0 release.

## Features

- **System information** — OS, kernel, architecture, online CPU count, and username
- **Host information** — hostname and Linux distribution
- **CPU** — processor model, architecture, and online processor count
- **GPU** — graphics device vendor and device IDs, plus driver name when available
- **Memory** — total and available RAM
- **Load average** — 1-, 5-, and 15-minute load averages
- **Uptime** — system uptime in a readable format
- **Processes** — current process count
- **Users** — logged-in terminal sessions
- **Networking** — IPv4 and IPv6 addresses for network interfaces
- **Disk usage** — total, used, and available space on the root filesystem
- **File fetching** — match file paths or glob patterns and report combined file size
- **Colored output** — configurable ANSI-colored section headings
- **Combined report** — display supported system information with one command

## Requirements

- Linux
- GCC or Clang
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

## Installation

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

Display help:

```bash
cinfo --help
```

Display the installed version:

```bash
cinfo --version
```

Display all supported information:

```bash
cinfo --all
```

### Command-line options

| Option | Description |
|---|---|
| `-h`, `--help` | Display help |
| `--version` | Display the cinfo version |
| `--sysinfo` | General system information |
| `--host` | Hostname and Linux distribution |
| `--cpu` | CPU model, architecture, and online processor count |
| `--gpu` | GPU device IDs and driver information |
| `--kernel` | Kernel information |
| `--memory` | Total and available RAM |
| `--load` | System load averages |
| `--uptime` | System uptime |
| `--processes` | Current process count |
| `--users` | Logged-in terminal sessions |
| `--network` | Network interface IP addresses |
| `--disk` | Root filesystem usage |
| `--fetch PATTERN` | List matching files and report their combined size |
| `--color=MODE` | Configure colored output |
| `--all` | Display all supported system information |

### File fetching

Pass a file path or a quoted glob pattern to `--fetch`:

```bash
cinfo --fetch ./README.md
cinfo --fetch './src/*.c'
```

Quote glob patterns so the shell passes the pattern to `cinfo` for expansion. The command reports matching regular files and their combined size.

### Color output

Use one of these modes with `--color=MODE`:

| Mode | Behavior |
|---|---|
| `auto` | Automatically detect whether standard output is a terminal; respect a non-empty `NO_COLOR` variable |
| `always` | Always emit ANSI color codes |
| `never` | Disable colored output |

Examples:

```bash
cinfo --color=auto --all
cinfo --color=always --cpu
cinfo --color=never --all
```

The default mode is `auto`.

> The `--fetch` and `--color=MODE` options are available in the v1.2.0-rc2 pre-release. They are not part of the stable `main` branch's command-line interface.

## Arch Linux package

A `PKGBUILD` is included in the repository. Build and install the package locally with:

```bash
makepkg -si
```

The package recipe downloads a versioned source archive from GitHub and verifies it using SHA-256.

## Releases

See the [GitHub Releases page](https://github.com/unknownember/cinfo/releases) for available versions.

The [v1.2.0-rc2 pre-release](https://github.com/unknownember/cinfo/releases/tag/v1.2.0-rc2) includes the file-fetching and configurable color-output features described above. It is for testing and is not the stable v1.2.0 release.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).

## Inspiration

Inspired by [mrdotx's cinfo](https://github.com/mrdotx/cinfo).
