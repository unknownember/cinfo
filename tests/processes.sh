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
    if (argc != 2)
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

echo 'All process regression tests passed.'
