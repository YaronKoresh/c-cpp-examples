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
if command -v wine >/dev/null 2>&1; then
  for t in backbuffer invalidate brush; do
    "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/${t}_test.exe" "tests/${t}_test.c" -lgdi32 -luser32
    if command -v xvfb-run >/dev/null 2>&1; then
      xvfb-run -a wine "${TMPDIR:-/tmp}/${t}_test.exe" 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok
    else
      wine "${TMPDIR:-/tmp}/${t}_test.exe" 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok
    fi
  done
fi
