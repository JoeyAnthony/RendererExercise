#include "w_window.h"
#include "logger.h"

#ifndef UNICODE
#define UNICODE
#endif

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ApplicationWindow::ApplicationWindow()
{
}

ApplicationWindow::~ApplicationWindow()
{
}

void ApplicationWindow::Initialize(const WindowConfig& config)
{
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = config.window_name.c_str();

    RegisterClass(&wc);

    // Create the window.
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    if (config.windowed) {

    }

    HWND hwnd = CreateWindowEx(
        0,
        config.window_name.c_str(),
        config.window_text.c_str(),
        windowStyle,

        // Size and position
        config.pos_X, config.pos_y, config.size_x, config.size_y,

        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        LOG << "Window creation failed: " << GetLastError();
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
}

void ApplicationWindow::Destroy()
{
}

void ApplicationWindow::UpdateWindow()
{
    // Run the message loop.

    MSG msg = { };
    if (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

