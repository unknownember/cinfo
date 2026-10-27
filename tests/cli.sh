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

# --fetch expands quoted globs and reports file sizes.
mkdir "$tmp/fetch"
printf 'abc' > "$tmp/fetch/one.iso"
printf '12345' > "$tmp/fetch/two.iso"

if ./cinfo --fetch "$tmp/fetch/*.iso" >"$tmp/out" 2>"$tmp/err"; then
    grep -Fq 'one.iso' "$tmp/out" &&
    grep -Fq 'two.iso' "$tmp/out" &&
    grep -Fq 'Total size: 8 B' "$tmp/out" ||
    {
        printf 'FAIL: --fetch output or total size was incorrect\n' >&2
        cat "$tmp/out" >&2
        exit 1
    }
    pass "--fetch expands glob and totals file sizes"
else
    printf 'FAIL: --fetch returned an error\n' >&2
    cat "$tmp/err" >&2
    exit 1
fi


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

check_error "missing --fetch pattern" --fetch
check_error "invalid option" --definitely-invalid
check_error "extra CPU argument" --cpu extra
check_error "extra help argument" --help extra
check_error "extra version argument" --version extra
check_error "double dash" --
check_error "single dash" -


# Color regression checks
esc=$(printf '\033')

if output=$(./cinfo --color=always --sysinfo 2>&1) &&
   printf '%s' "$output" | grep -Fq "$esc"; then
    echo "PASS: --color=always emits ANSI escapes"
else
    echo "FAIL: --color=always did not emit ANSI escapes"
    exit 1
fi

if output=$(./cinfo --color=never --sysinfo 2>&1) &&
   ! printf '%s' "$output" | grep -Fq "$esc"; then
    echo "PASS: --color=never disables ANSI escapes"
else
    echo "FAIL: --color=never emitted ANSI escapes"
    exit 1
fi

if output=$(./cinfo --color=auto --sysinfo 2>&1) &&
   ! printf '%s' "$output" | grep -Fq "$esc"; then
    echo "PASS: --color=auto disables color when piped"
else
    echo "FAIL: --color=auto emitted ANSI escapes when piped"
    exit 1
fi

if ./cinfo --color=invalid --sysinfo >/dev/null 2>&1; then
    echo "FAIL: invalid color mode was accepted"
    exit 1
else
    status=$?
    if [ "$status" -eq 2 ]; then
        echo "PASS: invalid color mode exits with status 2"
    else
        echo "FAIL: invalid color mode exited with status $status"
        exit 1
    fi
fi

if command -v script >/dev/null 2>&1; then
    if output=$(script -qec './cinfo --color=auto --sysinfo' /dev/null 2>/dev/null) &&
       printf '%s' "$output" | grep -Fq "$esc"; then
        echo "PASS: --color=auto emits color on a terminal"
    else
        echo "FAIL: --color=auto did not emit color on a terminal"
        exit 1
    fi

    if output=$(NO_COLOR=1 script -qec './cinfo --color=auto --sysinfo' /dev/null 2>/dev/null) &&
       ! printf '%s' "$output" | grep -Fq "$esc"; then
        echo "PASS: NO_COLOR disables automatic terminal color"
    else
        echo "FAIL: NO_COLOR was not respected on a terminal"
        exit 1
    fi
else
    echo "SKIP: terminal color checks (script command unavailable)"
fi

printf '\nAll CLI regression checks passed.\n'
