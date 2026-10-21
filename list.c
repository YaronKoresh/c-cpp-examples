#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "shape_list.h"
#include "backbuffer.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddControls(HWND hwnd);
void DrawShapes(HDC hdc);
void AddShape(int x, int y, int size, COLORREF color, int shapeType);
void RemoveShape(int index);
void ClearShapes(HWND hwnd);
void PlaceControl(HWND control, const LayoutBox *box);
void UpdateListScrollExtent(void);

HWND hButtonAdd;
HWND hButtonRemove;
HWND hButtonClear;
HWND hEdit;
HWND hList;
HDC hMemDC;
HBITMAP hBitmap;
int clientWidth, clientHeight;
ListLayout layout;
bool isDrawing = false;
int currentShapeType = 0;
int penStyle = 0;
int lastDrawX = 0, lastDrawY = 0;
unsigned long drawingColor = 0;

Shape shapes[MAX_SHAPES];
int numShapes = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    srand(time(NULL));

    const wchar_t CLASS_NAME[] = L"MyWindowClass";
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Window registration failed!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"My colorful list",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"Window creation failed!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN && hEdit && msg.hwnd == hEdit) {
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1001, BN_CLICKED), (LPARAM)hButtonAdd);
            continue;
        }
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE && hEdit && msg.hwnd == hEdit) {
            SetWindowText(hEdit, L"");
            continue;
        }
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_DELETE && hList && msg.hwnd == hList) {
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1003, BN_CLICKED), (LPARAM)hButtonRemove);
            continue;
        }
        /* Lets Tab and Shift+Tab move the focus between the controls that have WS_TABSTOP. */
        if (IsDialogMessage(hwnd, &msg)) {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void AddShape(int x, int y, int size, COLORREF color, int shapeType) {
    if (MakeRoomForShape(shapes, &numShapes)) {
        shapes[numShapes].x = x;
        shapes[numShapes].y = y;
        shapes[numShapes].size = size;
        shapes[numShapes].color = color;
        shapes[numShapes].shapeType = shapeType;
        shapes[numShapes].listed = 0;
        numShapes++;
    }
}

void RemoveShape(int index) {
    RemoveShapeAt(shapes, &numShapes, index);
}

void ClearShapes(HWND hwnd) {
    numShapes = 0;
    InvalidateOwnWindow(hwnd);
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
                ReplaceBackBuffer(&hMemDC, &hBitmap, GetDC(hwnd), clientWidth, clientHeight);
            }
            layout = ComputeListLayout(clientWidth, clientHeight);
            PlaceControl(hEdit, &layout.edit);
            PlaceControl(hList, &layout.list);
            PlaceControl(hButtonAdd, &layout.addButton);
            PlaceControl(hButtonRemove, &layout.removeButton);
            PlaceControl(hButtonClear, &layout.clearButton);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                int textLength = GetWindowTextLength(hEdit);
                wchar_t *buffer = (wchar_t *)malloc((textLength + 1) * sizeof(wchar_t));
                if (!buffer) {
                    break;
                }
                GetWindowText(hEdit, buffer, textLength + 1);
                if (MakeRoomForShape(shapes, &numShapes)) {
                    int listed = HasVisibleWideText(buffer);
                    if (listed) {
                        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
                        UpdateListScrollExtent();
                        SetWindowText(hEdit, L"");
                    }
                    shapes[numShapes].size = (rand() % 50) + 20;
                    RandomShapePosition(&layout.canvas, shapes[numShapes].size,
                                        &shapes[numShapes].x, &shapes[numShapes].y);
                    shapes[numShapes].color = RGB(rand() % 256, rand() % 256, rand() % 256);
                    shapes[numShapes].shapeType = rand() % 2;
                    shapes[numShapes].listed = listed;
                    numShapes++;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    MessageBox(hwnd, L"The list is full. Remove an item first.", L"Add Item and Shape",
                               MB_OK | MB_ICONINFORMATION);
                }
                free(buffer);
            }
            else if (LOWORD(wParam) == 1003) {
                int selectedIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (selectedIndex != LB_ERR) {
                    RemoveShape(ShapeIndexForListItem(shapes, numShapes, selectedIndex));
                    int remaining = (int)SendMessage(hList, LB_DELETESTRING, selectedIndex, 0);
                    /* keeps a selection, so that Delete can be pressed again for the next item */
                    if (remaining != LB_ERR && remaining > 0) {
                        SendMessage(hList, LB_SETCURSEL, selectedIndex < remaining ? selectedIndex : remaining - 1, 0);
                    }
                    UpdateListScrollExtent();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            else if (LOWORD(wParam) == 1004) {
                ClearShapes(hwnd);
                SendMessage(hList, LB_RESETCONTENT, 0, 0);
                UpdateListScrollExtent();
            }
            break;
        case WM_PAINT: {
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
            DrawShapes(hMemDC);

            BitBlt(hdc,
                ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
                hMemDC,
                ps.rcPaint.left, ps.rcPaint.top,
                SRCCOPY);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN:
            isDrawing = true;
            lastDrawX = MouseXFromLParam(lParam);
            lastDrawY = MouseYFromLParam(lParam);
            /* a click without a move still leaves one shape */
            AddShape(lastDrawX, lastDrawY, 10, drawingColor, currentShapeType);
            InvalidateRect(hwnd, NULL, FALSE);
            SetCapture(hwnd);
            break;
        case WM_LBUTTONUP:
            isDrawing = false;
            ReleaseCapture();
            break;
        case WM_RBUTTONDOWN:
            /* a right click steps to the next color and shape of the freehand pen */
            penStyle = (penStyle + 1) % PEN_STYLES;
            PenStyleAt(penStyle, &drawingColor, &currentShapeType);
            break;
        case WM_CAPTURECHANGED:
            isDrawing = false;
            break;
        case WM_MOUSEMOVE:
            if (isDrawing && LeftButtonHeld(wParam)) {
                int x = MouseXFromLParam(lParam);
                int y = MouseYFromLParam(lParam);
                /* fills the way from the last point to this one, so a fast move leaves no gaps */
                int steps = StrokeSteps(lastDrawX, lastDrawY, x, y, 10);
                for (int i = 1; i <= steps; i++) {
                    AddShape(lastDrawX + (x - lastDrawX) * i / steps, lastDrawY + (y - lastDrawY) * i / steps, 10,
                             drawingColor, currentShapeType);
                }
                lastDrawX = x;
                lastDrawY = y;
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
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
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
        L"Arial"
    );
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hList = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"LISTBOX",
        L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY,
        10, 40, 780, 450,
        hwnd,
        (HMENU)1002,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hButtonAdd = CreateWindowEx(
        0,
        L"BUTTON",
        L"Add Item and Shape",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        10, 500, 150, 30,
        hwnd,
        (HMENU)1001,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hButtonAdd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hButtonRemove = CreateWindowEx(
        0,
        L"BUTTON",
        L"Remove Item",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        170, 500, 150, 30,
        hwnd,
        (HMENU)1003,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hButtonRemove, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    hButtonClear = CreateWindowEx(
        0,
        L"BUTTON",
        L"Clear All",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        330, 500, 150, 30,
        hwnd,
        (HMENU)1004,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hButtonClear, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

void DrawShapes(HDC hdc) {
    for (int i = 0; i < numShapes; i++) {
        DrawShapeOnDC(hdc, &shapes[i]);
    }
}

void PlaceControl(HWND control, const LayoutBox *box) {
    if (control) {
        MoveWindow(control, box->x, box->y, box->width, box->height, TRUE);
    }
}

/* Sets the width the list box can scroll sideways to the width of its widest item, so that
   items wider than the list box can be read. */
void UpdateListScrollExtent(void) {
    HDC hdc = GetDC(hList);
    HFONT font = (HFONT)SendMessage(hList, WM_GETFONT, 0, 0);
    HGDIOBJ oldFont = font ? SelectObject(hdc, font) : NULL;
    int count = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
    int widest = 0;
    for (int i = 0; i < count; i++) {
        int length = (int)SendMessage(hList, LB_GETTEXTLEN, i, 0);
        if (length < 0) {
            continue;
        }
        wchar_t *text = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
        if (!text) {
            continue;
        }
        SendMessage(hList, LB_GETTEXT, i, (LPARAM)text);
        SIZE size;
        if (GetTextExtentPoint32(hdc, text, length, &size) && size.cx > widest) {
            widest = size.cx;
        }
        free(text);
    }
    if (oldFont) {
        SelectObject(hdc, oldFont);
    }
    ReleaseDC(hList, hdc);
    SendMessage(hList, LB_SETHORIZONTALEXTENT, widest + 8, 0);
}
