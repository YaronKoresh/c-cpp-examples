#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddControls(HWND hwnd);

HWND hEdit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "MyWindowClass";

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window registration failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "My First Textbox",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Window creation failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            AddControls(hwnd);
            break;

        case WM_SIZE:
            if (hEdit) {
                int newWidth = LOWORD(lParam);
                int newHeight = HIWORD(lParam);
                SetWindowPos(hEdit, NULL, 10, 10, newWidth - 20, newHeight - 20, SWP_NOZORDER);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                if (HIWORD(wParam) == EN_CHANGE) {
                    char buffer[256];
                    GetWindowText(hEdit, buffer, sizeof(buffer));
                    printf("Edit control text: %s\n", buffer);
                }
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void AddControls(HWND hwnd) {
    hEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        10, 10, 780, 580,
        hwnd,
        (HMENU)1001,
        GetModuleHandle(NULL),
        NULL
    );
    if (hEdit == NULL){
        MessageBox(hwnd, "Could not create edit control", "Error", MB_OK);
        return;
    }
    HFONT hFont = CreateFont(
        16,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        "Arial"
    );
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}
