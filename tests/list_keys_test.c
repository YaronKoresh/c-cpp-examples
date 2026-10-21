#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Starts list.exe and checks the keyboard shortcuts: typing reaches the edit control and
   Delete removes the selected list item. */
static HWND window, edit, listBox;

static int ListCount(void) {
    return (int)SendMessage(listBox, LB_GETCOUNT, 0, 0);
}

static void AddItem(const char *text) {
    SetWindowText(edit, text);
    SendMessage(window, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), 0);
    Sleep(200);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: list_keys_test list.exe");
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
    SetForegroundWindow(window);
    edit = FindWindowEx(window, NULL, "EDIT", NULL);
    listBox = FindWindowEx(window, NULL, "LISTBOX", NULL);
    AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(window, NULL), TRUE);
    int failed = 0;

    /* typed letters still reach the edit control */
    SetFocus(edit);
    Sleep(200);
    keybd_event('Q', 0, 0, 0);
    keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
    Sleep(300);
    char typed[16] = "";
    SendMessage(edit, WM_GETTEXT, sizeof(typed), (LPARAM)typed);
    if (_stricmp(typed, "q") != 0) {
        puts("FAIL: a typed letter did not reach the edit control");
        failed = 1;
    }

    /* Delete removes the selected item */
    AddItem("first");
    AddItem("second");
    AddItem("third");
    SendMessage(listBox, LB_SETCURSEL, 1, 0);
    SetFocus(listBox);
    Sleep(200);
    keybd_event(VK_DELETE, 0, 0, 0);
    keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
    Sleep(300);
    WCHAR text[16] = {0};
    if (ListCount() != 2) {
        puts("FAIL: Delete did not remove the selected item");
        failed = 1;
    } else {
        SendMessageW(listBox, LB_GETTEXT, 1, (LPARAM)text);
        if (lstrcmpW(text, L"third") != 0) {
            puts("FAIL: Delete removed the wrong item");
            failed = 1;
        }
    }

    /* the item that took the place of the removed one is selected, so Delete works again */
    if (SendMessage(listBox, LB_GETCURSEL, 0, 0) != 1) {
        puts("FAIL: no item is selected after Delete");
        failed = 1;
    }
    keybd_event(VK_DELETE, 0, 0, 0);
    keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
    Sleep(300);
    if (ListCount() != 1) {
        puts("FAIL: a second Delete did not remove the next item");
        failed = 1;
    } else if (SendMessage(listBox, LB_GETCURSEL, 0, 0) != 0) {
        puts("FAIL: the last item is not selected after Delete");
        failed = 1;
    }
    keybd_event(VK_DELETE, 0, 0, 0);
    keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
    Sleep(300);
    if (ListCount() != 0) {
        puts("FAIL: Delete did not remove the last item");
        failed = 1;
    }
    AddItem("again");
    AddItem("more");

    /* Delete with nothing selected removes nothing */
    SendMessage(listBox, LB_SETCURSEL, (WPARAM)-1, 0);
    keybd_event(VK_DELETE, 0, 0, 0);
    keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
    Sleep(300);
    if (ListCount() != 2) {
        puts("FAIL: Delete without a selection changed the list");
        failed = 1;
    }

    TerminateProcess(pi.hProcess, 0);
    if (!failed) {
        puts("ok");
    }
    return failed;
}
