#include <windows.h>
#include <stdio.h>
#include "../backbuffer.h"

static int eraseA, eraseB;

static LRESULT CALLBACK ProcA(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_ERASEBKGND) eraseA++;
    return DefWindowProc(h, m, w, l);
}

static LRESULT CALLBACK ProcB(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_ERASEBKGND) eraseB++;
    return DefWindowProc(h, m, w, l);
}

static void Pump(void) {
    MSG msg;
    for (int i = 0; i < 50; i++) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }
}

int main(void) {
    HINSTANCE inst = GetModuleHandle(NULL);
    WNDCLASS a, b;
    ZeroMemory(&a, sizeof(a));
    ZeroMemory(&b, sizeof(b));
    a.lpfnWndProc = ProcA;
    a.lpszClassName = "InvA";
    a.hInstance = inst;
    b.lpfnWndProc = ProcB;
    b.lpszClassName = "InvB";
    b.hInstance = inst;
    RegisterClass(&a);
    RegisterClass(&b);
    HWND ha = CreateWindow("InvA", "a", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 200, 200, NULL, NULL, inst, NULL);
    HWND hb = CreateWindow("InvB", "b", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 250, 0, 200, 200, NULL, NULL, inst, NULL);
    (void)hb;
    Pump();

    eraseA = eraseB = 0;
    if (InvalidateOwnWindow(NULL)) {
        puts("FAIL: NULL window accepted");
        return 1;
    }
    Pump();
    if (eraseA != 0 || eraseB != 0) {
        puts("FAIL: other windows were repainted");
        return 1;
    }

    InvalidateOwnWindow(ha);
    Pump();
    if (eraseA == 0 || eraseB != 0) {
        puts("FAIL: only the given window must repaint");
        return 1;
    }

    puts("ok");
    return 0;
}
