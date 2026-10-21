#include <windows.h>
#include <stdio.h>

/* Starts list.exe and checks that a list item wider than the list box can be scrolled sideways. */
static HWND window, edit, listBox;

static void AddItem(const char *text) {
    SetWindowText(edit, text);
    SendMessage(window, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), 0);
    Sleep(200);
}

static int Extent(void) {
    return (int)SendMessage(listBox, LB_GETHORIZONTALEXTENT, 0, 0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: list_scroll_test list.exe");
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
    RECT box;
    GetClientRect(listBox, &box);
    int failed = 0;

    AddItem("short");
    if (Extent() > box.right) {
        puts("FAIL: a short item makes the list scroll sideways");
        failed = 1;
    }

    char longText[401];
    for (int i = 0; i < 400; i++) {
        longText[i] = (char)('a' + i % 26);
    }
    longText[400] = 0;
    AddItem(longText);
    int wide = Extent();
    if (!(GetWindowLong(listBox, GWL_STYLE) & WS_HSCROLL)) {
        puts("FAIL: the list box shows no horizontal scroll bar for a wide item");
        failed = 1;
    }
    if (wide <= box.right) {
        puts("FAIL: a 400 character item does not widen the scrollable area");
        failed = 1;
    }

    /* removing the long item (the second one) shrinks the scrollable area again */
    SendMessage(listBox, LB_SETCURSEL, 1, 0);
    SendMessage(window, WM_COMMAND, MAKEWPARAM(1003, BN_CLICKED), 0);
    Sleep(200);
    if (Extent() >= wide) {
        puts("FAIL: removing the long item keeps the wide scrollable area");
        failed = 1;
    }

    AddItem(longText);
    SendMessage(window, WM_COMMAND, MAKEWPARAM(1004, BN_CLICKED), 0);
    Sleep(200);
    if (Extent() > box.right) {
        puts("FAIL: Clear All keeps the wide scrollable area");
        failed = 1;
    }

    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
