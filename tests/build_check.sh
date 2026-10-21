#!/usr/bin/env bash
# Cross-compiles every example with mingw-w64 and rejects non-breaking spaces in sources.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-x86_64-w64-mingw32-gcc}"
if git rev-parse --git-dir >/dev/null 2>&1; then
  for f in hello.exe textbox.exe list.exe; do
    if ! git check-ignore -q "$f"; then
      echo "FAIL: $f is not ignored by git"
      exit 1
    fi
  done
fi
for f in *.c; do
  if grep -q $'\xc2\xa0' "$f"; then
    echo "FAIL: $f contains non-breaking spaces"
    exit 1
  fi
  "$CC" -Wall -Wextra -Werror -c "$f" -o /dev/null
  "$CC" -std=c11 -pedantic -Wall -Wextra -Werror -c "$f" -o /dev/null
  "${CXX:-x86_64-w64-mingw32-g++}" -Wall -Wextra -Werror -c -x c++ "$f" -o /dev/null
  echo "ok: $f"
done
for word in Enter Delete Tab; do
  grep -q "$word" README.md || { echo "FAIL: README.md does not mention $word"; exit 1; }
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
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list.exe" list.c -lgdi32 -luser32
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list_ui_test.exe" tests/list_ui_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine list_ui_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine list_ui_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list_tab_test.exe" tests/list_tab_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine list_tab_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine list_tab_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list_keys_test.exe" tests/list_keys_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine list_keys_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine list_keys_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list_layout_test.exe" tests/list_layout_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine list_layout_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine list_layout_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list_scroll_test.exe" tests/list_scroll_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine list_scroll_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine list_scroll_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -o "${TMPDIR:-/tmp}/list_draw_test.exe" tests/list_draw_test.c -luser32 -lgdi32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a -s "-screen 0 1280x800x24" wine list_draw_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine list_draw_test.exe list.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -Werror -mwindows -o "${TMPDIR:-/tmp}/textbox.exe" textbox.c
  "$CC" -Wall -Wextra -Werror -o "${TMPDIR:-/tmp}/textbox_ui_test.exe" tests/textbox_ui_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine textbox_ui_test.exe textbox.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine textbox_ui_test.exe textbox.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
  "$CC" -Wall -Wextra -Werror -o "${TMPDIR:-/tmp}/textbox_scroll_test.exe" tests/textbox_scroll_test.c -luser32
  if command -v xvfb-run >/dev/null 2>&1; then
    (cd "${TMPDIR:-/tmp}" && xvfb-run -a wine textbox_scroll_test.exe textbox.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  else
    (cd "${TMPDIR:-/tmp}" && wine textbox_scroll_test.exe textbox.exe 2>/dev/null | tr -d "\r" | tee /dev/stderr | grep -qx ok)
  fi
fi
