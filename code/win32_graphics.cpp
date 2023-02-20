
#include <math.h>
#include <windows.h>

#include "win32_graphics.h"
#include "graphics_math.cpp"

global global_state GlobalState;

v2 ProjectPoint(v3 WorldPos)
{
    v2 Result = {};
    // NOTE: Перетворюємо до NDC
    Result = WorldPos.xy / WorldPos.z;

    // NOTE: Перетворюємо до пікселів
    Result = 0.5f * (Result + V2(1.0f, 1.0f));
    Result = V2(GlobalState.FrameBufferWidth, GlobalState.FrameBufferHeight) * Result;
    
    return Result;
}

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
    LARGE_INTEGER TimerFrequency = {};
    Assert(QueryPerformanceFrequency(&TimerFrequency));

    {
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

        GlobalState.DeviceContext = GetDC(GlobalState.WindowHandle);
    }

    {
        //RECT ClientRect = {};
        //Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        //GlobalState.FrameBufferWidth = ClientRect.right - ClientRect.left;
        //GlobalState.FrameBufferHeight = ClientRect.bottom - ClientRect.top;

        GlobalState.FrameBufferWidth = 300;
        GlobalState.FrameBufferHeight = 300;
        GlobalState.FrameBufferPixels = (u32*)malloc(sizeof(u32) * GlobalState.FrameBufferWidth *
                                                     GlobalState.FrameBufferHeight);
    }

    LARGE_INTEGER BeginTime = {};
    LARGE_INTEGER EndTime = {};
    Assert(QueryPerformanceCounter(&BeginTime));
    
    while (GlobalState.IsRunning)
    {
        Assert(QueryPerformanceCounter(&EndTime));
        f32 FrameTime = f32(EndTime.QuadPart - BeginTime.QuadPart) / f32(TimerFrequency.QuadPart);
        BeginTime = EndTime;

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

        // NOTE: Очистуємо буфер кадри до 0
        for (u32 Y = 0; Y < GlobalState.FrameBufferHeight; ++Y)
        {
            for (u32 X = 0; X < GlobalState.FrameBufferWidth; ++X)
            {
                u32 PixelId = Y * GlobalState.FrameBufferWidth + X;

                u8 Red = 0;
                u8 Green = 0;
                u8 Blue = 0;
                u8 Alpha = 255;
                u32 PixelColor = ((u32)Alpha << 24) | ((u32)Red << 16) | ((u32)Green << 8) | (u32)Blue;
                
                GlobalState.FrameBufferPixels[PixelId] = PixelColor;
            }
        }

        // NOTE: Проєктуємо наші точки
        GlobalState.CurrTime = GlobalState.CurrTime + FrameTime;
        if (GlobalState.CurrTime > 2.0f * 3.14159f)
        {
            GlobalState.CurrTime -= 2.0f * 3.14159f;
        }
        
        for (u32 TriangleId = 0; TriangleId < 10; ++TriangleId)
        {
            f32 DistToCamera = powf(2.0f, TriangleId + 1);
            v3 Points[3] =
                {
                    V3(-1.0f, -0.5f, DistToCamera),
                    V3(1.0f, -0.5f, DistToCamera),
                    V3(0, 0.5f, DistToCamera),
                };

            for (u32 PointId = 0; PointId < ArrayCount(Points); ++PointId)
            {
                v3 ShiftedPoint = Points[PointId] + V3(cosf(GlobalState.CurrTime), sinf(GlobalState.CurrTime), 0);
                v2 PixelPos = ProjectPoint(ShiftedPoint);

                if (PixelPos.x >= 0.0f && PixelPos.x < GlobalState.FrameBufferWidth &&
                    PixelPos.y >= 0.0f && PixelPos.y < GlobalState.FrameBufferHeight)
                {
                    u32 PixelId = u32(PixelPos.y) * GlobalState.FrameBufferWidth + u32(PixelPos.x);
                    GlobalState.FrameBufferPixels[PixelId] = 0xFF00FF00;
                }
            }
        }
        
        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        u32 ClientWidth = ClientRect.right - ClientRect.left;
        u32 ClientHeight = ClientRect.bottom - ClientRect.top;

        BITMAPINFO BitmapInfo = {};
        BitmapInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
        BitmapInfo.bmiHeader.biWidth = GlobalState.FrameBufferWidth;
        BitmapInfo.bmiHeader.biHeight = GlobalState.FrameBufferHeight;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
        
        Assert(StretchDIBits(GlobalState.DeviceContext,
                             0,
                             0,
                             ClientWidth,
                             ClientHeight,
                             0,
                             0,
                             GlobalState.FrameBufferWidth,
                             GlobalState.FrameBufferHeight,
                             GlobalState.FrameBufferPixels,
                             &BitmapInfo,
                             DIB_RGB_COLORS,
                             SRCCOPY));
    }
    
    return 0;
}
