#pragma once
#include <windows.h>

class Window {
public:
    Window(int width, int height, const wchar_t* title);
    ~Window();
    bool Initialize();
    void Cleanup();
    HWND GetHandle() const { return hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    int width, height;
    const wchar_t* title;
    HWND hwnd;
};