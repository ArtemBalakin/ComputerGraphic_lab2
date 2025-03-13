#pragma once
#include <windows.h>

class Window {
public:
    Window(int width, int height, const wchar_t *title);

    ~Window();

    bool Initialize();

    void Show();

    void Cleanup();

    HWND GetHandle() const { return hwnd; }

private:
    int width, height;
    const wchar_t *title;
    HWND hwnd;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
