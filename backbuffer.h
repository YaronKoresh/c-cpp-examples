#ifndef BACKBUFFER_H
#define BACKBUFFER_H

#include <windows.h>
#include "shape_list.h"

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

/* Draws one shape with its own brush. The previous brush is selected back before the
   new brush is deleted, because GDI cannot delete a brush that is still selected in a DC. */
static inline void DrawShapeOnDC(HDC hdc, const Shape *shape) {
    HBRUSH brush = CreateSolidBrush(shape->color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);

    if (shape->shapeType == 0) {
        Rectangle(hdc, shape->x, shape->y, shape->x + shape->size, shape->y + shape->size);
    } else {
        Ellipse(hdc, shape->x, shape->y, shape->x + shape->size, shape->y + shape->size);
    }

    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

#endif
