#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'SRC'
#define main cinfo_program_main
#include "cc.c"
#undef main

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    return show_gpu_from(argv[1]);
}
SRC

cc -I "$repo_dir" -Wall -Wextra -Wpedantic -Werror -O0 \
    "$tmp/test.c" -o "$tmp/test"

fail() {
    echo "FAIL: $1" >&2
    exit 1
}

pass() {
    echo "PASS: $1"
}

make_gpu() {
    card=$1
    vendor=$2
    device=$3
    driver=$4
    base="$tmp/drm/$card/device"

    mkdir -p "$base"
    if [ "$vendor" != "-" ]; then
        printf '%s\n' "$vendor" > "$base/vendor"
    fi
    if [ "$device" != "-" ]; then
        printf '%s\n' "$device" > "$base/device"
    fi
    if [ "$driver" != "-" ]; then
        ln -s "/some/nested/driver/path/$driver" "$base/driver"
    fi
}

run_test() {
    set +e
    output=$("$tmp/test" "$1" 2>/dev/null)
    result=$?
    set -e
}

# One valid GPU.
mkdir -p "$tmp/drm"
make_gpu card0 0x8086 0x0406 i915
run_test "$tmp/drm"
[ "$result" -eq 0 ] || fail "valid GPU exit status"
[ "$output" = "GPU (card0): vendor 0x8086, device 0x0406, driver i915" ] ||
    fail "valid GPU output"
pass "valid GPU"

# Multiple GPUs and connector entries.
make_gpu card1 0x10de 0x1c82 nvidia
mkdir -p "$tmp/drm/card0-DP-1/device"
printf '0x9999\n' > "$tmp/drm/card0-DP-1/device/vendor"
printf '0x9999\n' > "$tmp/drm/card0-DP-1/device/device"
mkdir -p "$tmp/drm/card1junk/device"
printf '0x9999\n' > "$tmp/drm/card1junk/device/vendor"
printf '0x9999\n' > "$tmp/drm/card1junk/device/device"
run_test "$tmp/drm"
[ "$result" -eq 0 ] || fail "multiple GPU exit status"
printf '%s\n' "$output" | grep -Fq \
    'GPU (card0): vendor 0x8086, device 0x0406, driver i915' ||
    fail "first GPU missing"
printf '%s\n' "$output" | grep -Fq \
    'GPU (card1): vendor 0x10de, device 0x1c82, driver nvidia' ||
    fail "second GPU missing"
[ "$(printf '%s\n' "$output" | grep -c '^GPU (')" -eq 2 ] ||
    fail "connector entry incorrectly reported as a GPU"
pass "multiple GPUs; connector ignored"

# Missing vendor and missing device entries are skipped.
rm -rf "$tmp/drm"
mkdir -p "$tmp/drm"
make_gpu card0 - 0x0406 i915
make_gpu card1 0x10de - nvidia
run_test "$tmp/drm"
[ "$result" -eq 0 ] || fail "incomplete GPU entry exit status"
[ "$output" = "GPU:          No DRM GPU devices found" ] ||
    fail "incomplete GPU entries should be skipped"
pass "missing vendor/device skipped"

# Missing driver symlink should be reported as unknown.
rm -rf "$tmp/drm"
mkdir -p "$tmp/drm"
make_gpu card0 0x8086 0x0406 -
run_test "$tmp/drm"
[ "$result" -eq 0 ] || fail "missing driver exit status"
[ "$output" = "GPU (card0): vendor 0x8086, device 0x0406, driver unknown" ] ||
    fail "missing driver output"
pass "missing driver link"

# Missing DRM directory should fail.
run_test "$tmp/does-not-exist"
[ "$result" -eq 1 ] || fail "missing directory should return 1"
pass "missing DRM directory"

echo "All GPU regression tests passed."
