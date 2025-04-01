#include "Window.h"



Window::Window(int width, int height, const wchar_t* title)

    : hwnd(nullptr), width(width), height(height) {

    std::cout << "[Window] Constructor called: width=" << width << ", height=" << height << std::endl;

}



Window::~Window() {

    if (hwnd) {

        DestroyWindow(hwnd);

        std::cout << "[Window] Window destroyed" << std::endl;

    }

}



bool Window::Initialize() {

    std::cout << "[Window] Initializing window..." << std::endl;



    WNDCLASS wc = {};

    wc.lpfnWndProc = DefWindowProc;

    wc.hInstance = GetModuleHandle(nullptr);

    wc.lpszClassName = L"CG_Lab1_WindowClass";

    if (!RegisterClass(&wc)) {

        std::cerr << "[Window] Failed to register window class" << std::endl;

        return false;

    }



    hwnd = CreateWindowEx(

        0, L"CG_Lab1_WindowClass", L"CG_Lab1",

        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,

        width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr

    );

    if (!hwnd) {

        std::cerr << "[Window] Failed to create window" << std::endl;

        return false;

    }



    ShowWindow(hwnd, SW_SHOW);

    std::cout << "[Window] Window created successfully: hwnd=" << hwnd << std::endl;

    return true;

}