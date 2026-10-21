#include <windows.h>
#include <stdio.h>

/* Starts list.exe and checks that Tab and Shift+Tab move the keyboard focus between the controls. */
static HWND FocusOf(HWND window) {
    GUITHREADINFO info;
    info.cbSize = sizeof(info);
    GetGUIThreadInfo(GetWindowThreadProcessId(window, NULL), &info);
    return info.hwndFocus;
}

static void PressTab(int shift) {
    if (shift) {
        keybd_event(VK_SHIFT, 0, 0, 0);
    }
    keybd_event(VK_TAB, 0, 0, 0);
    keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
    if (shift) {
        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
    }
    Sleep(300);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: list_tab_test list.exe");
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
    SetForegroundWindow(window);
    Sleep(300);
    HWND edit = FindWindowEx(window, NULL, "EDIT", NULL);
    HWND listBox = FindWindowEx(window, NULL, "LISTBOX", NULL);
    HWND addButton = FindWindowEx(window, NULL, "BUTTON", NULL);
    AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(window, NULL), TRUE);
    SetFocus(edit);
    Sleep(200);

    int failed = 0;
    if (FocusOf(window) != edit) {
        puts("FAIL: cannot focus the edit control");
        failed = 1;
    }
    PressTab(0);
    if (FocusOf(window) != listBox) {
        puts("FAIL: Tab does not move the focus from the edit control to the list box");
        failed = 1;
    }
    PressTab(0);
    if (FocusOf(window) != addButton) {
        puts("FAIL: Tab does not move the focus from the list box to the first button");
        failed = 1;
    }
    PressTab(1);
    if (FocusOf(window) != listBox) {
        puts("FAIL: Shift+Tab does not move the focus back to the list box");
        failed = 1;
    }

    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
