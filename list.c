#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddControls(HWND hwnd);
void DrawShapes(HWND hwnd, HDC hdc);
void AddShape(int x, int y, int size, COLORREF color, int shapeType);
void RemoveShape(int index);
void ClearShapes();

HWND hButtonAdd;
HWND hButtonRemove;
HWND hButtonClear;
HWND hEdit;
HWND hList;
HDC hMemDC;
HBITMAP hBitmap;
int clientWidth, clientHeight;
bool isDrawing = false;
int currentShapeType = 0;
COLORREF drawingColor = RGB(0, 0, 0);

typedef struct {
    int x, y, size;
    COLORREF color;
    int shapeType;
} Shape;

#define MAX_SHAPES 100
Shape shapes[MAX_SHAPES];
int numShapes = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand(time(NULL));

    const char CLASS_NAME[] = "MyWindowClass";
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window registration failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "My colorful list",
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

void AddShape(int x, int y, int size, COLORREF color, int shapeType) {
    if (numShapes < MAX_SHAPES) {
        shapes[numShapes].x = x;
        shapes[numShapes].y = y;
        shapes[numShapes].size = size;
        shapes[numShapes].color = color;
        shapes[numShapes].shapeType = shapeType;
        numShapes++;
    }
}

void RemoveShape(int index) {
    if (index >= 0 && index < numShapes) {
        for (int i = index; i < numShapes - 1; i++) {
            shapes[i] = shapes[i + 1];
        }
        numShapes--;
    }
}

void ClearShapes() {
    numShapes = 0;
    InvalidateRect(NULL, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            AddControls(hwnd);
            break;

        case WM_SIZE:
            clientWidth = LOWORD(lParam);
            clientHeight = HIWORD(lParam);
            if (hMemDC) {
                DeleteObject(SelectObject(hMemDC, NULL));
                DeleteDC(hMemDC);
                hMemDC = CreateCompatibleDC(GetDC(hwnd));
                hBitmap = CreateCompatibleBitmap(GetDC(hwnd), clientWidth, clientHeight);
                SelectObject(hMemDC, hBitmap);
            }
            if (hEdit) {
                MoveWindow(hEdit, 10, 10, clientWidth - 20, 20, TRUE);
            }
            if (hList) {
                MoveWindow(hList, 10, 40, clientWidth - 20, clientHeight - 150, TRUE);
            }
            if (hButtonAdd) {
                MoveWindow(hButtonAdd, 10, clientHeight - 100, 150, 30, TRUE);
            }
            if (hButtonRemove) {
                MoveWindow(hButtonRemove, 170, clientHeight - 100, 150, 30, TRUE);
            }
            if (hButtonClear) {
                MoveWindow(hButtonClear, 330, clientHeight - 100, 150, 30, TRUE);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                char buffer[256];
                GetWindowText(hEdit, buffer, sizeof(buffer));
                if (strlen(buffer) > 0) {
                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
                    SetWindowText(hEdit, "");
                }
                if (numShapes < MAX_SHAPES) {
                    shapes[numShapes].x = rand() % clientWidth;
                    shapes[numShapes].y = rand() % clientHeight;
                    shapes[numShapes].size = (rand() % 50) + 20;
                    shapes[numShapes].color = RGB(rand() % 256, rand() % 256, rand() % 256);
                    shapes[numShapes].shapeType = rand() % 2;
                    numShapes++;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            else if (LOWORD(wParam) == 1003) {
                int selectedIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (selectedIndex != LB_ERR) {
                    RemoveShape(selectedIndex);
                    SendMessage(hList, LB_DELETESTRING, selectedIndex, 0);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            else if (LOWORD(wParam) == 1004) {
                ClearShapes();
                SendMessage(hList, LB_RESETCONTENT, 0, 0);
            }
            break;
        case WM_PAINT:
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (!hMemDC) {
                hMemDC = CreateCompatibleDC(hdc);
                clientWidth = ps.rcPaint.right - ps.rcPaint.left;
                clientHeight = ps.rcPaint.bottom - ps.rcPaint.top;
                hBitmap = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
                SelectObject(hMemDC, hBitmap);
            }

            Rectangle(hMemDC, 0, 0, clientWidth, clientHeight);
            DrawShapes(hwnd, hMemDC);

            BitBlt(hdc,
                ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
                hMemDC,
                ps.rcPaint.left, ps.rcPaint.top,
                SRCCOPY);

            EndPaint(hwnd, &ps);
            break;
        case WM_LBUTTONDOWN:
            isDrawing = true;
            SetCapture(hwnd);
            break;
        case WM_LBUTTONUP:
            isDrawing = false;
            ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            if (isDrawing) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                AddShape(x, y, 10, drawingColor, currentShapeType);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_DESTROY:
            if (hMemDC) {
                DeleteObject(SelectObject(hMemDC, NULL));
                DeleteDC(hMemDC);
            }
            if (hBitmap) {
                DeleteObject(hBitmap);
            }
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
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        10, 10, 780, 20,
        hwnd,
        (HMENU)1000,
        GetModuleHandle(NULL),
        NULL
    );
    HFONT hFont = CreateFont(
        14,
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

    hList = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "LISTBOX",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        10, 40, 780, 450,
        hwnd,
        (HMENU)1002,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hButtonAdd = CreateWindowEx(
        0,
        "BUTTON",
        "Add Item and Shape",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 500, 150, 30,
        hwnd,
        (HMENU)1001,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hButtonAdd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hButtonRemove = CreateWindowEx(
        0,
        "BUTTON",
        "Remove Item",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        170, 500, 150, 30,
        hwnd,
        (HMENU)1003,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hButtonRemove, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hButtonClear = CreateWindowEx(
        0,
        "BUTTON",
        "Clear All",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        330, 500, 150, 30,
        hwnd,
        (HMENU)1004,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hButtonClear, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

void DrawShapes(HWND hwnd, HDC hdc) {
    for (int i = 0; i < numShapes; i++) {
        HBRUSH hBrush = CreateSolidBrush(shapes[i].color);
        SelectObject(hdc, hBrush);

        if (shapes[i].shapeType == 0) {
            Rectangle(hdc, shapes[i].x, shapes[i].y, shapes[i].x + shapes[i].size, shapes[i].y + shapes[i].size);
        } else {
            Ellipse(hdc, shapes[i].x, shapes[i].y, shapes[i].x + shapes[i].size, shapes[i].y + shapes[i].size);
        }
        DeleteObject(hBrush);
    }
}
