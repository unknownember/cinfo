#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/load-test.c" <<'C_EOF'
#define main cinfo_program_main
#include "cc.c"
#undef main

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    return show_load_from(argv[1]);
}
C_EOF

cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror \
    -O0 "$tmp/load-test.c" -o "$tmp/load-test"

pass=0
fail=0

check_success() {
    name=$1
    if "$tmp/load-test" "$tmp/loadavg" >"$tmp/stdout" 2>"$tmp/stderr" &&
       grep -Fq 'Load average: 1.00, 0.50, 0.25' "$tmp/stdout"; then
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
    if "$tmp/load-test" "$tmp/loadavg" >"$tmp/stdout" 2>"$tmp/stderr"; then
        printf 'FAIL: %s (unexpected success)\n' "$name"
        cat "$tmp/stdout" >&2
        fail=$((fail + 1))
    else
        printf 'PASS: %s\n' "$name"
        pass=$((pass + 1))
    fi
}

printf '1.00 0.50 0.25 2/100 1234\n' > "$tmp/loadavg"
check_success "valid load averages"

printf '1.00 nope 0.25\n' > "$tmp/loadavg"
check_failure "malformed load average"

printf '1.00 0.50\n' > "$tmp/loadavg"
check_failure "missing third load average"

printf '%s\n' '-1.00 0.50 0.25' > "$tmp/loadavg"
check_failure "negative load average"

printf 'nan 0.50 0.25\n' > "$tmp/loadavg"
check_failure "NaN load average"

printf '1.00 inf 0.25\n' > "$tmp/loadavg"
check_failure "infinite load average"

if "$tmp/load-test" "$tmp/missing-file" >"$tmp/stdout" 2>"$tmp/stderr"; then
    printf 'FAIL: missing input file (unexpected success)\n'
    fail=$((fail + 1))
else
    printf 'PASS: missing input file\n'
    pass=$((pass + 1))
fi

printf '\nLoad tests: %s passed, %s failed\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
