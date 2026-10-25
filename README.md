# cinfo

A tiny, fast Linux system information tool written in pure C.

## Features

- **System information:** OS, kernel, architecture, online CPU count, and username
- **CPU:** architecture and online processor count
- **Kernel:** kernel version
- **Memory:** total and available RAM
- **Load:** 1-, 5-, and 15-minute system load averages
- **Disk:** root filesystem capacity, used space, and available space
- **Combined report:** display all supported information at once

## Requirements

- Linux
- A C compiler (such as GCC or Clang)
- GNU Make
- glibc

## Build

Clone the repository and compile:

```bash
git clone https://github.com/unknownember/cinfo.git
cd cinfo
make
```

Run it directly from the project directory:

```bash
./cinfo --all
```

## Install

Install to `/usr/bin/cinfo`:

```bash
sudo make install
```

To remove the executable:

```bash
sudo rm /usr/bin/cinfo
```

## Usage

```text
cinfo --help
cinfo --version
cinfo --sysinfo
cinfo --cpu
cinfo --kernel
cinfo --memory
cinfo --load
cinfo --disk
cinfo --all
```

| Option | Description |
|---|---|
| `-h`, `--help` | Show help |
| `--version` | Show version |
| `--sysinfo` | General system information |
| `--cpu` | CPU architecture and online CPU count |
| `--kernel` | Kernel version |
| `--memory` | RAM usage information |
| `--load` | System load averages |
| `--disk` | Root filesystem usage |
| `--all` | Display all information |

## Arch Linux package

An Arch `PKGBUILD` is included in this repository. To build a package locally:

```bash
makepkg -si
```

The package recipe downloads a versioned source archive from GitHub and verifies it using SHA-256.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).

## Source

https://github.com/unknownember/cinfo
