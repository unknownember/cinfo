#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/memory-test.c" <<'EOF'
#define main cinfo_program_main
#include "cc.c"
#undef main

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    return show_memory_from(argv[1]);
}
EOF

cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror \
    -O0 "$tmp/memory-test.c" -o "$tmp/memory-test"

pass=0
fail=0

check_success() {
    name=$1
    expected_total=$2
    expected_available=$3
    if output=$("$tmp/memory-test" "$tmp/meminfo" 2>"$tmp/stderr") &&
       printf '%s\n' "$output" | grep -Fq "$expected_total" &&
       printf '%s\n' "$output" | grep -Fq "$expected_available"; then
        printf 'PASS: %s\n' "$name"
        pass=$((pass + 1))
    else
        printf 'FAIL: %s\n' "$name"
        cat "$tmp/stderr" >&2
        fail=$((fail + 1))
    fi
}

check_failure() {
    name=$1
    if "$tmp/memory-test" "$tmp/meminfo" >"$tmp/stdout" 2>"$tmp/stderr"; then
        printf 'FAIL: %s (unexpected success)\n' "$name"
        cat "$tmp/stdout" >&2
        fail=$((fail + 1))
    else
        printf 'PASS: %s\n' "$name"
        pass=$((pass + 1))
    fi
}

printf 'MemTotal: 2097152 kB\nMemAvailable: 1048576 kB\n' > "$tmp/meminfo"
check_success "valid memory information" "2.00 GiB" "1.00 GiB"

printf 'MemTotal: 2097152 kB\n' > "$tmp/meminfo"
check_failure "missing MemAvailable"

printf 'MemAvailable: 1048576 kB\n' > "$tmp/meminfo"
check_failure "missing MemTotal"

printf 'MemTotal: 2097152 MB\nMemAvailable: 1048576 kB\n' > "$tmp/meminfo"
check_failure "invalid MemTotal unit"

printf 'MemTotal: nope kB\nMemAvailable: 1048576 kB\n' > "$tmp/meminfo"
check_failure "malformed MemTotal value"

printf 'MemTotal: -1 kB\nMemAvailable: 1048576 kB\n' > "$tmp/meminfo"
check_failure "negative MemTotal value"

printf 'MemTotal: 2097152 kB\nMemAvailable: -1 kB\n' > "$tmp/meminfo"
check_failure "negative MemAvailable value"

printf 'MemTotal: 2097152 kB\nMemAvailable: nope kB\n' > "$tmp/meminfo"
check_failure "malformed MemAvailable value"

if "$tmp/memory-test" "$tmp/does-not-exist" >"$tmp/stdout" 2>"$tmp/stderr"; then
    printf 'FAIL: missing input file (unexpected success)\n'
    fail=$((fail + 1))
else
    printf 'PASS: missing input file\n'
    pass=$((pass + 1))
fi

printf '\nMemory tests: %s passed, %s failed\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
