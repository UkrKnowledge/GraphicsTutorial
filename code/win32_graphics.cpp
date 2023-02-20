
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

f32 CrossProduct2d(v2 A, v2 B)
{
    f32 Result = A.x * B.y - A.y * B.x;
    return Result;
}

void DrawTriangle(v3* Points, u32 Color)
{
    v2 PointA = ProjectPoint(Points[0]);
    v2 PointB = ProjectPoint(Points[1]);
    v2 PointC = ProjectPoint(Points[2]);

    v2 Edge0 = PointB - PointA;
    v2 Edge1 = PointC - PointB;
    v2 Edge2 = PointA - PointC;

    b32 IsTopLeft0 = (Edge0.x >= 0.0f && Edge0.y > 0.0f) || (Edge0.x > 0.0f && Edge0.y == 0.0f);
    b32 IsTopLeft1 = (Edge1.x >= 0.0f && Edge1.y > 0.0f) || (Edge1.x > 0.0f && Edge1.y == 0.0f);
    b32 IsTopLeft2 = (Edge2.x >= 0.0f && Edge2.y > 0.0f) || (Edge2.x > 0.0f && Edge2.y == 0.0f);
    
    for (u32 Y = 0; Y < GlobalState.FrameBufferHeight; ++Y)
    {
        for (u32 X = 0; X < GlobalState.FrameBufferWidth; ++X)
        {
            v2 PixelPoint = V2(X, Y) + V2(0.5f, 0.5f);

            v2 PixelEdge0 = PixelPoint - PointA;
            v2 PixelEdge1 = PixelPoint - PointB;
            v2 PixelEdge2 = PixelPoint - PointC;

            f32 CrossLength0 = CrossProduct2d(PixelEdge0, Edge0);
            f32 CrossLength1 = CrossProduct2d(PixelEdge1, Edge1);
            f32 CrossLength2 = CrossProduct2d(PixelEdge2, Edge2);
            
            if ((CrossLength0 > 0.0f || (IsTopLeft0 && CrossLength0 == 0.0f)) &&
                (CrossLength1 > 0.0f || (IsTopLeft1 && CrossLength1 == 0.0f)) &&
                (CrossLength2 > 0.0f || (IsTopLeft2 && CrossLength2 == 0.0f)))
            {
                // NOTE: Ми у середині трикутника
                u32 PixelId = Y * GlobalState.FrameBufferWidth + X;
                GlobalState.FrameBufferPixels[PixelId] = Color;
            }
        }
    }
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

        GlobalState.FrameBufferWidth = 50;
        GlobalState.FrameBufferHeight = 50;
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

        // NOTE: Проєктуємо наші трикутники
        GlobalState.CurrTime = GlobalState.CurrTime + FrameTime;
        if (GlobalState.CurrTime > 2.0f * 3.14159f)
        {
            GlobalState.CurrTime -= 2.0f * 3.14159f;
        }

        u32 Colors[] =
        {
            0xFF00FF00,
            0xFFFF00FF,
            0xFF0000FF,
        };

        v3 Points1[3] =
        {
            V3(-1.0f, -1.0f, 1.0f),
            V3(-1.0f,  1.0f, 1.0f),
            V3( 1.0f,  1.0f, 1.0f),
        };

        v3 Points2[3] =
        {
            V3( 1.0f,  1.0f, 1.0f),
            V3( 1.0f, -1.0f, 1.0f),
            V3(-1.0f, -1.0f, 1.0f),
        };

        DrawTriangle(Points1, Colors[0]);
        DrawTriangle(Points2, Colors[1]);
        
#if 0
        for (i32 TriangleId = 9; TriangleId >= 0; --TriangleId)
        {
            f32 DistToCamera = powf(2.0f, TriangleId + 1);
            v3 Points[3] =
                {
                    V3(-1.0f, -0.5f, DistToCamera),
                    V3(0, 0.5f, DistToCamera),
                    V3(1.0f, -0.5f, DistToCamera),
                };

            // NOTE: Рухаємо точки трикутника в коло
            for (u32 PointId = 0; PointId < ArrayCount(Points); ++PointId)
            {
                v3 ShiftedPoint = Points[PointId] + V3(cosf(GlobalState.CurrTime), sinf(GlobalState.CurrTime), 0);
                Points[PointId] = ShiftedPoint;
            }

            DrawTriangle(Points, Colors[TriangleId % ArrayCount(Colors)]);
        }
#endif
        
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
