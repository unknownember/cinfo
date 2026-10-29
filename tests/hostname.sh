#!/bin/sh
set -eu
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'C'
#define main cinfo_main
#include "cc.c"
#undef main

int main(void)
{
    char valid[] = "thinkpad-l440";
    if (validate_hostname(valid, sizeof(valid)) != 0)
        return 1;

    char empty[] = "";
    errno = 0;
    if (validate_hostname(empty, sizeof(empty)) != -1 || errno != EINVAL)
        return 2;

    char truncated[] = {'t', 'h', 'i', 'n', 'k'};
    errno = 0;
    if (validate_hostname(truncated, sizeof(truncated)) != -1 ||
        errno != ENAMETOOLONG)
        return 3;

    errno = 0;
    if (validate_hostname(valid, 0) != -1 || errno != ENAMETOOLONG)
        return 4;

    return 0;
}
C

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror -O0 "$tmp/test.c" -o "$tmp/test"

if "$tmp/test"; then
    echo 'PASS: valid hostname accepted'
    echo 'PASS: empty hostname rejected'
    echo 'PASS: unterminated/truncated hostname rejected'
    echo 'PASS: zero-sized hostname buffer rejected'
else
    echo 'FAIL: hostname validation regression checks' >&2
    exit 1
fi

echo 'All hostname regression tests passed.'
