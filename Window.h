#ifndef WINDOW_H
#define WINDOW_H
#include <Windows.h>

class Window {
public:
    bool Initialize(HINSTANCE hInstance, int width, int height, const wchar_t* title);
    void Cleanup();
    HWND GetHWND() const { return hwnd; }
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND hwnd = nullptr;
};
#endif