#include <windows.h>
#include <stdio.h>

/* Starts textbox.exe, fills it with 100 lines and checks that the edit control shows scroll bars. */
int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: textbox_scroll_test textbox.exe");
        return 2;
    }
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcess(argv[1], NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        puts("FAIL: cannot start textbox.exe");
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
    HWND edit = FindWindowEx(window, NULL, "EDIT", NULL);
    if (!edit) {
        puts("FAIL: edit control not found");
        return 1;
    }
    char text[100 * 8 + 1] = "";
    for (int i = 0; i < 100; i++) {
        char line[16];
        sprintf(line, "line %d\r\n", i);
        lstrcatA(text, line);
    }
    SendMessage(edit, WM_SETTEXT, 0, (LPARAM)text);
    Sleep(300);

    /* a visible scroll bar takes width or height from the client area, on top of the borders */
    int failed = 0;
    RECT client, outer;
    GetClientRect(edit, &client);
    GetWindowRect(edit, &outer);
    if (outer.right - outer.left - client.right < GetSystemMetrics(SM_CXVSCROLL)) {
        puts("FAIL: the edit control shows no vertical scroll bar for 100 lines");
        failed = 1;
    }
    if (outer.bottom - outer.top - client.bottom < GetSystemMetrics(SM_CYHSCROLL)) {
        puts("FAIL: the edit control shows no horizontal scroll bar");
        failed = 1;
    }
    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
