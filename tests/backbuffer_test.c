#include <windows.h>
#include <stdio.h>
#include "../backbuffer.h"

int main(void) {
    HDC screen = GetDC(NULL);
    HDC memDC = NULL;
    HBITMAP bitmap = NULL;

    ReplaceBackBuffer(&memDC, &bitmap, screen, 400, 300);
    HBITMAP first = bitmap;
    if (!memDC || !first) {
        puts("FAIL: first buffer not created");
        return 1;
    }

    ReplaceBackBuffer(&memDC, &bitmap, screen, 410, 310);
    if (GetObjectType(first) == OBJ_BITMAP) {
        puts("FAIL: old bitmap leaked after resize");
        return 1;
    }

    ReplaceBackBuffer(&memDC, &bitmap, screen, 0, 0);
    if (!memDC || !bitmap) {
        puts("FAIL: zero-size buffer not created");
        return 1;
    }

    puts("ok");
    return 0;
}
