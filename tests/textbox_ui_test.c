#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Starts textbox.exe with its output in a file, types 400 letters and four Hebrew letters and
   checks that the last line printed by the example holds all of them, as UTF-8. */
#define TYPED 400
#define HEBREW "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d"
#define LABEL "Edit control text: "

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("usage: textbox_ui_test textbox.exe");
        return 2;
    }
    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
    HANDLE out = CreateFile("textbox_out.txt", GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
    if (out == INVALID_HANDLE_VALUE) {
        puts("FAIL: cannot create the output file");
        return 1;
    }
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = out;
    si.hStdError = out;
    if (!CreateProcess(argv[1], NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
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
    for (int i = 0; i < TYPED; i++) {
        SendMessage(edit, WM_CHAR, (WPARAM)('a' + i % 26), 0);
    }
    const WCHAR hebrew[] = {0x5e9, 0x5dc, 0x5d5, 0x5dd};
    for (int i = 0; i < 4; i++) {
        SendMessageW(edit, WM_CHAR, (WPARAM)hebrew[i], 0);
    }
    SendMessage(window, WM_CLOSE, 0, 0);
    if (WaitForSingleObject(pi.hProcess, 10000) != WAIT_OBJECT_0) {
        TerminateProcess(pi.hProcess, 0);
    }
    CloseHandle(out);

    static char text[1 << 20];
    FILE *file = fopen("textbox_out.txt", "rb");
    size_t size = file ? fread(text, 1, sizeof(text) - 1, file) : 0;
    if (file) {
        fclose(file);
    }
    text[size] = 0;

    char *last = NULL;
    for (char *p = text; (p = strstr(p, LABEL)) != NULL; p += strlen(LABEL)) {
        last = p;
    }
    if (!last) {
        puts("FAIL: the example printed no text");
        return 1;
    }
    last += strlen(LABEL);
    size_t length = strcspn(last, "\r\n");
    if (length != TYPED + strlen(HEBREW)) {
        printf("FAIL: the last printed text has %d bytes instead of %d\n", (int)length, (int)(TYPED + strlen(HEBREW)));
        return 1;
    }
    if (strncmp(last + TYPED, HEBREW, strlen(HEBREW)) != 0) {
        puts("FAIL: the Hebrew letters were not printed as UTF-8");
        return 1;
    }
    puts("ok");
    return 0;
}
