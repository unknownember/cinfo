#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/uptime-test.c" <<'C_EOF'
#define main cinfo_program_main
#include "cc.c"
#undef main

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    return show_uptime_from(argv[1]);
}
C_EOF

cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror \
    -O0 "$tmp/uptime-test.c" -o "$tmp/uptime-test"

pass=0
fail=0

check_success() {
    name=$1
    expected=$2
    if "$tmp/uptime-test" "$tmp/uptime" >"$tmp/stdout" 2>"$tmp/stderr" &&
       grep -Fq "$expected" "$tmp/stdout"; then
        printf 'PASS: %s\n' "$name"
        pass=$((pass + 1))
    else
        printf 'FAIL: %s\n' "$name"
        cat "$tmp/stderr" >&2
        cat "$tmp/stdout" >&2
        fail=$((fail + 1))
    fi
}

check_failure() {
    name=$1
    if "$tmp/uptime-test" "$tmp/uptime" >"$tmp/stdout" 2>"$tmp/stderr"; then
        printf 'FAIL: %s (unexpected success)\n' "$name"
        cat "$tmp/stdout" >&2
        fail=$((fail + 1))
    else
        printf 'PASS: %s\n' "$name"
        pass=$((pass + 1))
    fi
}

printf '65.9 12345.0\n' > "$tmp/uptime"
check_success "valid fractional uptime" \
    'Uptime: 0 hours, 1 minutes, 5 seconds'

printf '90061.0 12345.0\n' > "$tmp/uptime"
check_success "uptime including days" \
    'Uptime: 1 days, 1 hours, 1 minutes, 1 seconds'

printf 'not-a-number 12345.0\n' > "$tmp/uptime"
check_failure "malformed uptime"

printf '%s\n' '-1.0 12345.0' > "$tmp/uptime"
check_failure "negative uptime"

printf 'nan 12345.0\n' > "$tmp/uptime"
check_failure "NaN uptime"

printf 'inf 12345.0\n' > "$tmp/uptime"
check_failure "infinite uptime"

printf '1e300 12345.0\n' > "$tmp/uptime"
check_failure "out-of-range uptime"

if "$tmp/uptime-test" "$tmp/missing-file" >"$tmp/stdout" 2>"$tmp/stderr"; then
    printf 'FAIL: missing input file (unexpected success)\n'
    fail=$((fail + 1))
else
    printf 'PASS: missing input file\n'
    pass=$((pass + 1))
fi

printf '\nUptime tests: %s passed, %s failed\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
