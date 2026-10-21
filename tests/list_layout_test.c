#include <windows.h>
#include <stdio.h>

/* Starts list.exe, makes the window narrow and checks that every button is inside the client area. */
int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: list_layout_test list.exe");
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
    SetWindowPos(window, NULL, 0, 0, 300, 250, SWP_NOMOVE | SWP_NOZORDER);
    Sleep(500);
    RECT client;
    GetClientRect(window, &client);
    int failed = 0;
    int buttons = 0;
    for (HWND button = NULL; (button = FindWindowEx(window, button, "BUTTON", NULL)) != NULL;) {
        RECT r;
        GetWindowRect(button, &r);
        POINT topLeft = {r.left, r.top}, bottomRight = {r.right, r.bottom};
        ScreenToClient(window, &topLeft);
        ScreenToClient(window, &bottomRight);
        buttons++;
        if (topLeft.x < 0 || bottomRight.x > client.right) {
            printf("FAIL: button %d (x %ld to %ld) is outside the client width %ld\n", buttons, topLeft.x,
                   bottomRight.x, client.right);
            failed = 1;
        }
    }
    if (buttons != 3) {
        puts("FAIL: expected three buttons");
        failed = 1;
    }
    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
