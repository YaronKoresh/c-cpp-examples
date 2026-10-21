#ifndef BACKBUFFER_H
#define BACKBUFFER_H

#include <windows.h>

/* Replaces the off-screen buffer with a new one of the given size.
   The old memory DC and the old bitmap are both deleted. */
static inline void ReplaceBackBuffer(HDC *memDC, HBITMAP *bitmap, HDC windowDC, int width, int height) {
    if (*memDC) {
        DeleteDC(*memDC);
    }
    if (*bitmap) {
        DeleteObject(*bitmap);
    }
    if (width < 1) {
        width = 1;
    }
    if (height < 1) {
        height = 1;
    }
    *memDC = CreateCompatibleDC(windowDC);
    *bitmap = CreateCompatibleBitmap(windowDC, width, height);
    SelectObject(*memDC, *bitmap);
}

/* Repaints one window. A NULL window is refused, because InvalidateRect(NULL, ...)
   would invalidate and redraw every window on the desktop. */
static inline BOOL InvalidateOwnWindow(HWND hwnd) {
    if (!hwnd) {
        return FALSE;
    }
    return InvalidateRect(hwnd, NULL, TRUE);
}

#endif
