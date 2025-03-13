#include "Window.h"

Window::Window(int width, int height, const wchar_t *title)
    : width(width), height(height), title(title), hwnd(nullptr) {
}

Window::~Window() {
    Cleanup();
}

bool Window::Initialize() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"DirectXWindowClass";
    if (!RegisterClass(&wc)) return false;

    hwnd = CreateWindow(L"DirectXWindowClass", title, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                        nullptr, nullptr, GetModuleHandle(nullptr), this);
    if (!hwnd) return false;

    return true;
}

void Window::Show() {
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
}

void Window::Cleanup() {
    if (hwnd) DestroyWindow(hwnd);
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams));
    }

    Window *window = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window) {
        switch (msg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
