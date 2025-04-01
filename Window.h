#pragma once
#include <windows.h>
#include <iostream>

class Window {
public:
    Window(int width, int height, const wchar_t* title);
    ~Window();
    bool Initialize();
    HWND GetHandle() const { return hwnd; }

private:
    HWND hwnd;
    int width, height;
};