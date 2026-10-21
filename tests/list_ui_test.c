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

    /* Hebrew text stays Hebrew in the list box, whatever the ANSI code page is */
    const WCHAR hebrew[] = {0x5e9, 0x5dc, 0x5d5, 0x5dd, 0};
    SetWindowTextW(edit, hebrew);
    PressAdd();
    WCHAR listed[16] = {0};
    SendMessageW(listBox, LB_GETTEXT, 0, (LPARAM)listed);
    if (ListCount() != 1 || lstrcmpW(listed, hebrew) != 0) {
        puts("FAIL: Hebrew text did not reach the list box");
        failed = 1;
    }
    SendMessage(listBox, LB_RESETCONTENT, 0, 0);
    SendMessage(window, WM_COMMAND, MAKEWPARAM(1004, BN_CLICKED), 0);
    Sleep(200);

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

    /* Enter in the edit box adds the item like the button, and an empty box adds nothing */
    SetWindowText(edit, "entered");
    PostMessage(edit, WM_KEYDOWN, VK_RETURN, 0);
    Sleep(300);
    char left[32] = "x";
    SendMessage(edit, WM_GETTEXT, sizeof(left), (LPARAM)left);
    if (ListCount() != 3 || left[0] != 0) {
        puts("FAIL: Enter did not add the item");
        failed = 1;
    }
    PostMessage(edit, WM_KEYDOWN, VK_RETURN, 0);
    Sleep(300);
    if (ListCount() != 3) {
        puts("FAIL: Enter in an empty box added an item");
        failed = 1;
    }

    /* Delete in the list box removes the selected item like the button, and nothing without a selection */
    SendMessage(listBox, LB_SETCURSEL, 0, 0);
    PostMessage(listBox, WM_KEYDOWN, VK_DELETE, 0);
    Sleep(300);
    if (ListCount() != 2) {
        puts("FAIL: Delete did not remove the selected item");
        failed = 1;
    }
    SendMessage(listBox, LB_SETCURSEL, (WPARAM)-1, 0);
    PostMessage(listBox, WM_KEYDOWN, VK_DELETE, 0);
    Sleep(300);
    if (ListCount() != 2) {
        puts("FAIL: Delete without a selection removed an item");
        failed = 1;
    }

    /* a full list tells the user why Add does nothing */
    while (ListCount() < 100) {
        SetWindowText(edit, "item");
        SendMessage(window, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), 0);
    }
    SetWindowText(edit, "one too many");
    PostMessage(window, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), 0);
    HWND notice = NULL;
    for (int i = 0; i < 30 && !notice; i++) {
        Sleep(100);
        notice = FindWindow("#32770", "Add Item and Shape");
    }
    if (!notice) {
        puts("FAIL: no message when the list is full");
        failed = 1;
    } else {
        PostMessage(notice, WM_CLOSE, 0, 0);
        Sleep(300);
    }
    char kept[32] = "";
    SendMessage(edit, WM_GETTEXT, sizeof(kept), (LPARAM)kept);
    if (ListCount() != 100 || strcmp(kept, "one too many") != 0) {
        puts("FAIL: a full list changed the list or the text");
        failed = 1;
    }

    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
