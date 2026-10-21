# c-cpp-examples

Small Win32 programs written in C, for learning.

| File | What it shows |
| --- | --- |
| `hello.c` | A message box. |
| `textbox.c` | A window with a multiline edit control that follows the window size. |
| `list.c` | A list box, buttons (also Enter and Delete), freehand drawing and a double-buffered window. |
| `shape_list.h` | Window-free helpers for the shape list used by `list.c`. |
| `backbuffer.h` | GDI helpers for the off-screen buffer, shapes and repainting. |

## Build

Cross-compile with mingw-w64 (Debian or Ubuntu: `sudo apt-get install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64`):

```sh
x86_64-w64-mingw32-gcc -mwindows -o hello.exe hello.c
x86_64-w64-mingw32-gcc -mwindows -o textbox.exe textbox.c
x86_64-w64-mingw32-gcc -mwindows -o list.exe list.c -lgdi32
```

On Windows with MinGW the same commands work with `gcc`.

## Test

```sh
bash tests/build_check.sh
```

The script compiles every example (as C, as strict C11 and as C++) with `-Wall -Wextra -Werror`, rejects non-breaking spaces in the sources and runs the unit
tests for `shape_list.h`. When `wine` is installed (and `xvfb-run`, for a virtual display) it also
runs the GDI tests and drives `list.exe` and `textbox.exe` through their controls.
