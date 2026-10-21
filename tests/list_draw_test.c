#include <windows.h>
#include <stdio.h>

/* Starts list.exe, drags the mouse 100 pixels in one message and checks that the line is
   continuous: the pixel in the middle of the drag must be drawn. */
static COLORREF ScreenPixel(HWND window, int x, int y) {
    POINT point = {x, y};
    ClientToScreen(window, &point);
    HDC screen = GetDC(NULL);
    COLORREF color = GetPixel(screen, point.x, point.y);
    ReleaseDC(NULL, screen);
    return color;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: list_draw_test list.exe");
        return 2;
    }
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcess(argv[1], NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        puts("FAIL: cannot start list.exe");
        return 1;
    }
    HWND window = NULL;
    for (int i = 0; i < 100 && !window; i++) {
        Sleep(100);
        window = FindWindow("MyWindowClass", NULL);
    }
    if (!window) {
        puts("FAIL: window not found");
        return 1;
    }
    Sleep(800);
    int failed = 0;
    if (ScreenPixel(window, 650, 300) != RGB(255, 255, 255)) {
        puts("FAIL: the canvas is not white before drawing");
        failed = 1;
    }
    SendMessage(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(600, 300));
    SendMessage(window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(700, 300));
    SendMessage(window, WM_LBUTTONUP, 0, MAKELPARAM(700, 300));
    Sleep(500);
    if (ScreenPixel(window, 650, 305) == RGB(255, 255, 255)) {
        puts("FAIL: a fast drag leaves a gap in the line");
        failed = 1;
    }
    if (ScreenPixel(window, 650, 340) != RGB(255, 255, 255)) {
        puts("FAIL: the line is drawn far from the mouse path");
        failed = 1;
    }
    /* a drag of 19 pixels is not a multiple of the shape size 10 and must still be continuous */
    SendMessage(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(600, 200));
    SendMessage(window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(619, 200));
    SendMessage(window, WM_LBUTTONUP, 0, MAKELPARAM(619, 200));
    Sleep(500);
    if (ScreenPixel(window, 612, 205) == RGB(255, 255, 255)) {
        puts("FAIL: a drag that is not a multiple of the shape size leaves a gap");
        failed = 1;
    }
    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
