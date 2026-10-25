#!/bin/sh
set -eu

if [ ! -x ./cinfo ]; then
    echo "FAIL: ./cinfo not found; run make first" >&2
    exit 1
fi

check() {
    name=$1
    expected=$2
    shift 2
    set +e
    ./cinfo "$@" >/dev/null 2>&1
    actual=$?
    set -e
    if [ "$actual" -eq "$expected" ]; then
        printf 'PASS: %-24s exit=%s\n' "$name" "$actual"
    else
        printf 'FAIL: %-24s expected=%s actual=%s\n' "$name" "$expected" "$actual" >&2
        exit 1
    fi
}

check "no arguments" 0
for opt in --sysinfo --help --version --cpu --kernel --memory --load --disk --uptime --host --processes --gpu --users --network --all; do
    check "$opt" 0 "$opt"
done
check "invalid option" 2 --definitely-invalid
check "extra CPU argument" 2 --cpu extra
check "extra help argument" 2 --help extra
check "extra version argument" 2 --version extra
check "double dash" 2 --
check "single dash" 2 -

printf '\nAll 21 CLI checks passed.\n'
