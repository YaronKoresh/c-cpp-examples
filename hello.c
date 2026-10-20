#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MessageBox(
        NULL,
        "Hello world!",
        "Message",
        MB_OK
    );

    return 0;
}