# cinfo

**A tiny, fast Linux system information tool written in pure C.**

`cinfo` is a command-line utility that displays useful information about your Linux system, including hardware, memory, processes, networking, and filesystem usage.

## Features

- **System information:** OS, kernel, architecture, online CPU count, and username
- **Host information:** hostname and Linux distribution
- **CPU:** processor model, architecture, and online processor count
- **GPU:** graphics device vendor and device IDs, and driver name when available
- **Memory:** total and available RAM
- **Load average:** 1-, 5-, and 15-minute system load averages
- **Uptime:** system uptime in a readable format
- **Processes:** current process count
- **Users:** logged-in terminal sessions
- **Networking:** IPv4 and IPv6 addresses for network interfaces
- **Disk:** total, used, and available space on the root filesystem
- **Combined report:** display all supported information with one command

## Requirements

- Linux
- A C compiler, such as GCC or Clang
- GNU Make
- glibc

## Build

Clone the repository and compile:

```bash
git clone https://github.com/unknownember/cinfo.git
cd cinfo
make
```

Run `cinfo` directly from the project directory:

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

To remove the locally built executable from the project directory:

```bash
make clean
```

## Usage

Run `cinfo --help` to display the available options.

```bash
cinfo --help
cinfo --version
cinfo --all
```

### Command-line options

| Option | Description |
|---|---|
| `-h`, `--help` | Display help |
| `--version` | Display the cinfo version |
| `--sysinfo` | General system information |
| `--host` | Hostname and distribution |
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
| `--all` | Display all supported information |

## Arch Linux package

An Arch Linux `PKGBUILD` is included in this repository.

To build and install the package locally:

```bash
makepkg -si
```

The package recipe downloads a versioned source archive from GitHub and verifies it using SHA-256.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).

## Source

[github.com/unknownember/cinfo](https://github.com/unknownember/cinfo)
