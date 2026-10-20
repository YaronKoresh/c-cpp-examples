#!/usr/bin/env bash
# Cross-compiles every example with mingw-w64 and rejects non-breaking spaces in sources.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-x86_64-w64-mingw32-gcc}"
for f in *.c; do
  if grep -q $'\xc2\xa0' "$f"; then
    echo "FAIL: $f contains non-breaking spaces"
    exit 1
  fi
  "$CC" -Wall -c "$f" -o /dev/null
  echo "ok: $f"
done
gcc -Wall -Wextra -o "${TMPDIR:-/tmp}/shape_list_test" tests/shape_list_test.c
"${TMPDIR:-/tmp}/shape_list_test"
