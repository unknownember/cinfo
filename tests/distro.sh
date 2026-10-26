#!/bin/sh
set -eu
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'C'
#define main cinfo_main
#include "cc.c"
#undef main

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;

    char distro[256];

    if (strcmp(argv[2], "--errno-enoent") == 0) {
        errno = 0;
        int result = read_distro(argv[1], distro);
        return result == -1 && errno == ENOENT ? 0 : 6;
    }

    int result = read_distro(argv[1], distro);
    if (result != 0)
        return result == 1 ? 3 : 4;
    return strcmp(distro, argv[2]) == 0 ? 0 : 5;
}
C

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror -O0 "$tmp/test.c" -o "$tmp/test"

check() {
    name=$1
    contents=$2
    expected=$3
    printf '%s\n' "$contents" > "$tmp/os-release"
    if "$tmp/test" "$tmp/os-release" "$expected"; then
        printf 'PASS: %s\n' "$name"
    else
        printf 'FAIL: %s\n' "$name" >&2
        exit 1
    fi
}

check "pretty name preferred" 'NAME="Fallback OS"
VERSION="2.0"
PRETTY_NAME="Example Linux"' 'Example Linux'
check "name plus version fallback" 'NAME="Example Linux"
VERSION="2.0"' 'Example Linux 2.0'
check "name only fallback" 'NAME="Example Linux"' 'Example Linux'
check "quoted escape decoding" 'PRETTY_NAME="Example \"Linux\""' 'Example "Linux"'

printf 'PRETTY_NAME=""\nNAME=""\n' > "$tmp/os-release"
if "$tmp/test" "$tmp/os-release" unused; then
    echo 'FAIL: empty values should be rejected' >&2
    exit 1
else
    result=$?
    [ "$result" -eq 3 ] || {
        echo "FAIL: empty values returned $result" >&2
        exit 1
    }
    echo 'PASS: empty values rejected'
fi


long_name=$(awk 'BEGIN { for (i = 0; i < 250; i++) printf "N" }')
long_version=$(awk 'BEGIN { for (i = 0; i < 250; i++) printf "V" }')
printf 'NAME="%s"\nVERSION="%s"\n' "$long_name" "$long_version" > "$tmp/os-release"
expected="${long_name} VVVV"
if "$tmp/test" "$tmp/os-release" "$expected"; then
    echo 'PASS: long fallback safely truncated'
else
    echo 'FAIL: long fallback safely truncated' >&2
    exit 1
fi

if "$tmp/test" "$tmp/does-not-exist" unused; then
    echo 'FAIL: missing file should return an error' >&2
    exit 1
else
    result=$?
    [ "$result" -eq 4 ] || {
        echo "FAIL: missing file returned $result" >&2
        exit 1
    }
    echo 'PASS: missing file rejected'
fi

if "$tmp/test" "$tmp/does-not-exist" --errno-enoent; then
    echo 'PASS: fopen errno preserved'
else
    echo 'FAIL: fopen errno was not preserved' >&2
    exit 1
fi

printf 'PRETTY_NAME="Broken\nNAME="Fallback OS"\n' > "$tmp/os-release"
if "$tmp/test" "$tmp/os-release" 'Fallback OS'; then
    echo 'PASS: malformed pretty name falls back to NAME'
else
    echo 'FAIL: malformed pretty name should fall back to NAME' >&2
    exit 1
fi

echo 'All distro regression tests passed.'
