#include "w_window.h"
#include "logger.h"

#ifndef UNICODE
#define UNICODE
#endif

std::map<unsigned long long, ApplicationWindow*> window_map;

LRESULT CALLBACK MS_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ApplicationWindow* window = window_map[(unsigned long long)hwnd];

    switch (uMsg)
    {
    case WM_CREATE:
        window_map[(unsigned long long)hwnd] = (ApplicationWindow*)lParam;
        break;
    default:
        return window->WindowProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT ApplicationWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ApplicationWindow::ApplicationWindow(): hinstance(NULL), hwindow(NULL), should_quit(false)
{
}

ApplicationWindow::~ApplicationWindow()
{
}

void ApplicationWindow::Initialize(const WindowConfig& config)
{
    WNDCLASS wc = { };

    wc.lpfnWndProc = MS_WindowProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = config.window_name.c_str();
    wc.cbClsExtra = sizeof(ApplicationWindow);

    RegisterClass(&wc);

    // Create the window.
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    if (config.windowed) {

    }

    hwindow = CreateWindowEx(
        0,
        config.window_name.c_str(),
        config.window_text.c_str(),
        windowStyle,

        // Size and position
        config.pos_X, config.pos_y, config.width, config.height,

        NULL,
        NULL,
        hinstance,
        this
    );

    if (hwindow == NULL)
    {
        LOG << "Window creation failed: " << GetLastError();
        return;
    }

    ShowWindow(hwindow, SW_SHOW);
}

void ApplicationWindow::Destroy()
{
}

void ApplicationWindow::UpdateWindow()
{
    // Run the message loop.
    MSG msg = { };
    if (PeekMessage(&msg, hwindow, 0, 0, PM_NOREMOVE)) {
        if (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

bool ApplicationWindow::GetShouldQuit()
{
    return should_quit;
}

