#!/usr/bin/env bash
# Tests the sketch on a computer. See test/README.md.
#
#   test/run.sh          compare with the last commit
#   test/run.sh main~2   compare with another commit, branch or tag
set -euo pipefail

ref="${1:-HEAD}"
here="$(cd "$(dirname "$0")" && pwd)"
repo="$(cd "$here/.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

# Switches on rings 2 and 3 in a copy of the sketch, so hours, minutes and
# seconds rings are all tested
enable_rings() {
  sed -i 's#^  // { SHOW_MINUTES#  { SHOW_MINUTES#; s#^  // { SHOW_SECONDS#  { SHOW_SECONDS#' "$1/FlameClock.ino"
}

# build <test file> <sketch folder> <program to make>
build() {
  g++ -std=gnu++11 -O1 -Wall -I"$here/stub" -DSKETCH="\"$2/FlameClock.ino\"" -x c++ "$here/$1" -o "$3"
}

echo "== Regression: the working folder against $ref =="
mkdir -p "$work/before" "$work/after/FlameClock" "$work/scenes/FlameClock"
git -C "$repo" archive "$ref" FlameClock | tar -x -C "$work/before"
cp "$repo"/FlameClock/* "$work/after/FlameClock/"
enable_rings "$work/before/FlameClock"
enable_rings "$work/after/FlameClock"

build regression.cpp "$work/before/FlameClock" "$work/regression-before"
build regression.cpp "$work/after/FlameClock" "$work/regression-after"
"$work/regression-before" > "$work/before.txt"
"$work/regression-after" > "$work/after.txt"
head -n 1 "$work/after.txt"

regression_ok=1
if cmp -s "$work/before.txt" "$work/after.txt"; then
  echo "IDENTICAL: $(grep -vc '^pair\|^rings' "$work/after.txt") frames"
else
  regression_ok=0
  changed=$(diff "$work/before.txt" "$work/after.txt" | grep -c '^>' || true)
  line=$(cmp "$work/before.txt" "$work/after.txt" | sed 's/.*line \([0-9]*\).*/\1/')
  pair=$(head -n "$line" "$work/after.txt" | grep '^pair' | tail -n 1 || true)
  echo "DIFFERENT: $changed frames changed. The first is at line $line ($pair, flame numbers from FLAME_LIST)."
  echo "If you changed a look or the drawing on purpose, this is expected."
fi

echo
echo "== Checks: the current sketch, with every scene switched on =="
cp "$repo"/FlameClock/* "$work/scenes/FlameClock/"
sed -i 's#^// \#define USE_\(SANCTUARY\|ASGARD\|POSEIDON\|HADES\)#\#define USE_\1#' "$work/scenes/FlameClock/Scenes.h"
build checks.cpp "$work/scenes/FlameClock" "$work/checks"
checks_ok=1
"$work/checks" || checks_ok=0

[ "$regression_ok" = 1 ] && [ "$checks_ok" = 1 ]
