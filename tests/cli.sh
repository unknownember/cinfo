#!/bin/sh
set -eu

if [ ! -x ./cinfo ]; then
    echo "FAIL: ./cinfo not found; run make first" >&2
    exit 1
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

pass() {
    printf 'PASS: %s\n' "$1"
}

check_output() {
    name=$1
    shift
    if ./cinfo "$@" >"$tmp/out" 2>"$tmp/err"; then
        if [ -s "$tmp/out" ]; then
            pass "$name (exit 0, non-empty output)"
        else
            printf 'FAIL: %s produced empty stdout\n' "$name" >&2
            exit 1
        fi
    else
        rc=$?
        printf 'FAIL: %s exited %s\n' "$name" "$rc" >&2
        cat "$tmp/err" >&2
        exit 1
    fi
}

check_output "no arguments"
for opt in --sysinfo --help --cpu --kernel --memory --load --disk --uptime --host --processes --gpu --users --network --all; do
    check_output "$opt" "$opt"
done

if ! ./cinfo --version >"$tmp/out" 2>"$tmp/err"; then
    printf 'FAIL: --version returned an error\n' >&2
    exit 1
fi
if ! grep -Fq '1.2.0-rc1' "$tmp/out"; then
    printf 'FAIL: --version did not report 1.2.0-rc1\n' >&2
    cat "$tmp/out" >&2
    exit 1
fi
pass "--version reports 1.2.0-rc1"

check_error() {
    name=$1
    shift
    set +e
    ./cinfo "$@" >"$tmp/out" 2>"$tmp/err"
    rc=$?
    set -e
    if [ "$rc" -eq 2 ] && [ -s "$tmp/err" ]; then
        pass "$name (exit 2, error message)"
    else
        printf 'FAIL: %s (exit=%s)\n' "$name" "$rc" >&2
        cat "$tmp/err" >&2
        exit 1
    fi
}

check_error "invalid option" --definitely-invalid
check_error "extra CPU argument" --cpu extra
check_error "extra help argument" --help extra
check_error "extra version argument" --version extra
check_error "double dash" --
check_error "single dash" -

printf '\nAll CLI regression checks passed.\n'
