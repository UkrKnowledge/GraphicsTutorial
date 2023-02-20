
#include <windows.h>

#include "win32_graphics.h"

global global_state GlobalState;

LRESULT Win32WindowCallBack(
  HWND WindowHandle,
  UINT Message,
  WPARAM WParam,
  LPARAM LParam
)
{
    LRESULT Result = {};

    switch (Message)
    {
        case WM_DESTROY:
        case WM_CLOSE:
        {
            GlobalState.IsRunning = false;
        } break;

        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nShowCmd)
{
    GlobalState.IsRunning = true;
    
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = Win32WindowCallBack;
    WindowClass.hInstance = hInstance;
    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.lpszClassName = "Graphics Tutorial";
    if (!RegisterClassA(&WindowClass))
    {
        InvalidCodePath;
    }

    GlobalState.WindowHandle = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "Graphics Tutorial",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!GlobalState.WindowHandle)
    {
        InvalidCodePath;
    }

    while (GlobalState.IsRunning)
    {
        MSG Message = {};
        while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
                case WM_QUIT:
                {
                    GlobalState.IsRunning = false;
                } break;

                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } break;
            }
        }
    }
    
    return 0;
}
