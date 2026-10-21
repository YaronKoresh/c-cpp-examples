#include <windows.h>
#include <stdio.h>
#include "../backbuffer.h"

int main(void) {
    HDC screen = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, 100, 100);
    SelectObject(memDC, bitmap);

    HGDIOBJ before = GetCurrentObject(memDC, OBJ_BRUSH);
    Shape square = {10, 10, 20, RGB(255, 0, 0), 0, 0};
    Shape circle = {40, 40, 20, RGB(0, 0, 255), 1, 0};
    DrawShapeOnDC(memDC, &square);
    DrawShapeOnDC(memDC, &circle);

    if (GetCurrentObject(memDC, OBJ_BRUSH) != before) {
        puts("FAIL: a shape brush is still selected, so it cannot be deleted");
        return 1;
    }
    if (GetPixel(memDC, 15, 15) != RGB(255, 0, 0) || GetPixel(memDC, 50, 50) != RGB(0, 0, 255)) {
        puts("FAIL: shapes were not drawn with their colors");
        return 1;
    }

    puts("ok");
    return 0;
}
