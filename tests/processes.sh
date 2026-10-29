#!/bin/sh
set -eu
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'C'
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>

static int inject_readdir_error;
static unsigned int readdir_calls;
static struct dirent *test_readdir(DIR *dir);

#define readdir test_readdir
#define main cinfo_main
#include "cc.c"
#undef main
#undef readdir

static struct dirent *test_readdir(DIR *dir)
{
    if (inject_readdir_error && readdir_calls++ >= 1) {
        errno = EIO;
        return NULL;
    }
    return readdir(dir);
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
        return 2;

    if (argc == 3 && strcmp(argv[2], "--read-error") == 0)
        inject_readdir_error = 1;
    else if (argc == 3)
        return 2;

    return show_processes_from(argv[1]);
}
C

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror -O0 "$tmp/test.c" -o "$tmp/test"

mkdir -p "$tmp/proc/1" "$tmp/proc/22" "$tmp/proc/007" "$tmp/proc/not-a-pid"
output=$("$tmp/test" "$tmp/proc")
if printf '%s\n' "$output" | grep -qx 'Processes:    3'; then
    echo 'PASS: counts numeric process entries only'
else
    printf 'FAIL: unexpected process count output: %s\n' "$output" >&2
    exit 1
fi

if "$tmp/test" "$tmp/missing" >/dev/null 2>&1; then
    echo 'FAIL: missing process directory should return an error' >&2
    exit 1
else
    echo 'PASS: missing process directory rejected'
fi

if "$tmp/test" "$tmp/proc" --read-error >/dev/null 2>&1; then
    echo 'FAIL: readdir error should return an error' >&2
    exit 1
else
    echo 'PASS: readdir error rejected'
fi

echo 'All process regression tests passed.'
