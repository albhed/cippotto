#!/usr/bin/env bash
set -u

pass=0
fail=0

check() {
    local description="$1"
    local result="$2"
    if [ "$result" -eq 0 ]; then
        echo "PASS: $description"
        pass=$((pass + 1))
    else
        echo "FAIL: $description"
        fail=$((fail + 1))
    fi
}

UNTOUCHED_FILES="src/chip8.c src/chip8.h src/display.c src/display.h src/input.c src/input.h src/audio.c src/audio.h src/loop.c src/loop.h src/quirks.c src/quirks.h"

git diff --quiet -- $UNTOUCHED_FILES
check "no changes" $?

[ -f src/main_web.c ]
check "src/main_web.c esiste" $?

if [ -f src/main_web.c ]; then
    grep -q "emscripten_set_main_loop_arg" src/main_web.c
    check "main_web.c usa emscripten_set_main_loop_arg" $?

    ! grep -Eq "while[[:space:]]*\(running\)|while[[:space:]]*\(true\)|while[[:space:]]*\(1\)" src/main_web.c
    check "main_web.c is not blocking" $?

    grep -q "parse_quirk_args" src/main_web.c
    check "main_web.c calls parse_quirk_args" $?

    grep -q "quirk_help_requested" src/main_web.c
    check "main_web.c calls quirk_help_requested" $?

    ! grep -q "strcmp(argv\[i\], \"--quirk" src/main_web.c
    check "main_web.c do not duplicate flags" $?
fi

if git diff --quiet -- Makefile; then
    build_test_unchanged=0
else
    removed_lines=$(git diff -- Makefile | grep -c '^-[^-]')
    [ "$removed_lines" -eq 0 ]
    build_test_unchanged=$?
fi
check "Makefile: only add" $build_test_unchanged

grep -Eq "^wasm:" Makefile
check "Makefile wasm target" $?

if grep -Eq "^wasm:" Makefile; then
    wasm_recipe=$(make -n wasm 2>&1)

    echo "$wasm_recipe" | grep -q "emcc"
    check "wasm target uses emcc" $?

    echo "$wasm_recipe" | grep -q -- "-sUSE_SDL=2"
    check "wasm target uses -sUSE_SDL=2" $?

    echo "$wasm_recipe" | grep -q -- "--preload-file roms"
    check "wasm target uses --preload-file roms" $?

    ! echo "$wasm_recipe" | grep -q "pkg-config"
    check "wasm target do not uses pkg-config" $?

    echo "$wasm_recipe" | grep -q "docs/chip8.js"
    check "wasm target produce docs/chip8.js" $?
fi

grep -qx "chip8.js" .gitignore
check ".gitignore ignore chip8.js" $?

grep -qx "chip8.wasm" .gitignore
check ".gitignore ignore chip8.wasm" $?

grep -qx "chip8.data" .gitignore
check ".gitignore ignore chip8.data" $?

echo ""
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
