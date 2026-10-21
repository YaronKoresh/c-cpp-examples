#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Starts list.exe, drives its controls with messages and checks the list box. */
static HWND window, edit, listBox;

static int ListCount(void) {
    return (int)SendMessage(listBox, LB_GETCOUNT, 0, 0);
}

static void PressAdd(void) {
    SendMessage(window, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), 0);
    Sleep(200);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: list_ui_test list.exe");
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
    for (int i = 0; i < 100 && !window; i++) {
        Sleep(100);
        window = FindWindow("MyWindowClass", NULL);
    }
    if (!window) {
        puts("FAIL: window not found");
        return 1;
    }
    Sleep(800);
    edit = FindWindowEx(window, NULL, "EDIT", NULL);
    listBox = FindWindowEx(window, NULL, "LISTBOX", NULL);

    int failed = 0;

    /* long text keeps every character */
    char longText[401];
    for (int i = 0; i < 400; i++) {
        longText[i] = (char)('a' + i % 26);
    }
    longText[400] = 0;
    SetWindowText(edit, longText);
    PressAdd();
    if (ListCount() != 1 || (int)SendMessage(listBox, LB_GETTEXTLEN, 0, 0) != 400) {
        puts("FAIL: long text was cut");
        failed = 1;
    }

    /* white space only adds no item */
    SetWindowText(edit, "  \t ");
    PressAdd();
    if (ListCount() != 1) {
        puts("FAIL: white space added an item");
        failed = 1;
    }

    /* an empty window does not break Add */
    ShowWindow(window, SW_MINIMIZE);
    Sleep(300);
    SetWindowText(edit, "after minimize");
    PressAdd();
    if (ListCount() != 2) {
        puts("FAIL: add after minimize");
        failed = 1;
    }

    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
