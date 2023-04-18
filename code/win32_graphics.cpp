
#include <cmath>
#include <windows.h>

#include "win32_graphics.h"
#include "graphics_math.cpp"
#include "clipper.cpp"

global global_state GlobalState;

v2 NdcToPixels(v2 NdcPos)
{
    v2 Result = {};
    Result = 0.5f * (NdcPos + V2(1.0f, 1.0f));
    Result = V2(GlobalState.FrameBufferWidth, GlobalState.FrameBufferHeight) * Result;
    
    return Result;
}

f32 CrossProduct2d(v2 A, v2 B)
{
    f32 Result = A.x * B.y - A.y * B.x;
    return Result;
}

v3 ColorU32ToRgb(u32 Color)
{
    v3 Result = {};
    Result.r = (Color >> 16) & 0xFF;
    Result.g = (Color >> 8) & 0xFF;
    Result.b = (Color >> 0) & 0xFF;
    Result /= 255.0f;
    return Result;
}

u32 ColorRgbToU32(v3 Color)
{
    Color *= 255.0f;
    u32 Result = ((u32)0xFF << 24) | ((u32)Color.r << 16) | ((u32)Color.g << 8) | (u32)Color.b;
    return Result;
}

void DrawTriangle(clip_vertex Vertex0, clip_vertex Vertex1, clip_vertex Vertex2, 
                  texture Texture, sampler Sampler)
{
    Vertex0.Pos.w = 1.0f / Vertex0.Pos.w;
    Vertex1.Pos.w = 1.0f / Vertex1.Pos.w;
    Vertex2.Pos.w = 1.0f / Vertex2.Pos.w;
    
    Vertex0.Pos.xyz *= Vertex0.Pos.w;
    Vertex1.Pos.xyz *= Vertex1.Pos.w;
    Vertex2.Pos.xyz *= Vertex2.Pos.w;
    
    v2 PointA = NdcToPixels(Vertex0.Pos.xy);
    v2 PointB = NdcToPixels(Vertex1.Pos.xy);
    v2 PointC = NdcToPixels(Vertex2.Pos.xy);

    i32 MinX = min(min((i32)PointA.x, (i32)PointB.x), (i32)PointC.x);
    i32 MaxX = max(max((i32)round(PointA.x), (i32)round(PointB.x)), (i32)round(PointC.x));
    i32 MinY = min(min((i32)PointA.y, (i32)PointB.y), (i32)PointC.y);
    i32 MaxY = max(max((i32)round(PointA.y), (i32)round(PointB.y)), (i32)round(PointC.y));

#if 0
    MinX = max(0, MinX);
    MinX = min(GlobalState.FrameBufferWidth - 1, MinX);
    MaxX = max(0, MaxX);
    MaxX = min(GlobalState.FrameBufferWidth - 1, MaxX);
    MinY = max(0, MinY);
    MinY = min(GlobalState.FrameBufferHeight - 1, MinY);
    MaxY = max(0, MaxY);
    MaxY = min(GlobalState.FrameBufferHeight - 1, MaxY);
#endif
    
    v2 Edge0 = PointB - PointA;
    v2 Edge1 = PointC - PointB;
    v2 Edge2 = PointA - PointC;

    b32 IsTopLeft0 = (Edge0.x >= 0.0f && Edge0.y > 0.0f) || (Edge0.x > 0.0f && Edge0.y == 0.0f);
    b32 IsTopLeft1 = (Edge1.x >= 0.0f && Edge1.y > 0.0f) || (Edge1.x > 0.0f && Edge1.y == 0.0f);
    b32 IsTopLeft2 = (Edge2.x >= 0.0f && Edge2.y > 0.0f) || (Edge2.x > 0.0f && Edge2.y == 0.0f);
    
    f32 BaryCentricDiv = CrossProduct2d(PointB - PointA, PointC - PointA);

    Vertex0.Uv *= Vertex0.Pos.w;
    Vertex1.Uv *= Vertex1.Pos.w;
    Vertex2.Uv *= Vertex2.Pos.w;

    f32 Edge0DiffX = Edge0.y;
    f32 Edge1DiffX = Edge1.y;
    f32 Edge2DiffX = Edge2.y;

    f32 Edge0DiffY = -Edge0.x;
    f32 Edge1DiffY = -Edge1.x;
    f32 Edge2DiffY = -Edge2.x;

    f32 Edge0RowY = CrossProduct2d(V2(MinX, MinY) - PointA, Edge0);
    f32 Edge1RowY = CrossProduct2d(V2(MinX, MinY) - PointB, Edge1);
    f32 Edge2RowY = CrossProduct2d(V2(MinX, MinY) - PointC, Edge2);
    
    for (i32 Y = MinY; Y <= MaxY; ++Y)
    {
        f32 Edge0 = Edge0RowY;
        f32 Edge1 = Edge1RowY;
        f32 Edge2 = Edge2RowY;
        
        for (i32 X = MinX; X <= MaxX; ++X)
        {
            if ((Edge0 > 0.0f || (IsTopLeft0 && Edge0 == 0.0f)) &&
                (Edge1 > 0.0f || (IsTopLeft1 && Edge1 == 0.0f)) &&
                (Edge2 > 0.0f || (IsTopLeft2 && Edge2 == 0.0f)))
            {
                // NOTE: Ми у середині трикутника
                u32 PixelId = Y * GlobalState.FrameBufferWidth + X;

                f32 T0 = -Edge1 / BaryCentricDiv;
                f32 T1 = -Edge2 / BaryCentricDiv;
                f32 T2 = -Edge0 / BaryCentricDiv;

                f32 DepthZ = T0 * Vertex0.Pos.z + T1 * Vertex1.Pos.z + T2 * Vertex2.Pos.z;
                if (DepthZ >= 0.0f && DepthZ <= 1.0f && DepthZ < GlobalState.DepthBuffer[PixelId])
                {
                    f32 OneOverW = T0 * Vertex0.Pos.w + T1 * Vertex1.Pos.w + T2 * Vertex2.Pos.w;

                    v2 Uv = T0 * Vertex0.Uv + T1 * Vertex1.Uv + T2 * Vertex2.Uv;
                    Uv /= OneOverW;

                    u32 TexelColor = 0;
                    switch (Sampler.Type)
                    {
                        case SamplerType_Nearest:
                        {
                            i32 TexelX = (i32)floorf(Uv.x * (Texture.Width - 1));
                            i32 TexelY = (i32)floorf(Uv.y * (Texture.Height - 1));
                            if (TexelX >= 0 && TexelX < Texture.Width &&
                                TexelY >= 0 && TexelY < Texture.Height)
                            {
                                TexelColor = Texture.Texels[TexelY * Texture.Width + TexelX];
                            }
                            else
                            {
                                TexelColor = 0xFF00FF00;
                            }
                        } break;

                        case SamplerType_Bilinear:
                        {
                            v2 TexelV2 = Uv * V2(Texture.Width, Texture.Height) - V2(0.5f, 0.5f);
                            v2i TexelPos[4] = {};
                            TexelPos[0] = V2I(floorf(TexelV2.x), floorf(TexelV2.y));
                            TexelPos[1] = TexelPos[0] + V2I(1, 0);
                            TexelPos[2] = TexelPos[0] + V2I(0, 1);
                            TexelPos[3] = TexelPos[0] + V2I(1, 1);

                            v3 TexelColors[4] = {};
                            for (u32 TexelId = 0; TexelId < ArrayCount(TexelPos); ++TexelId)
                            {
                                v2i CurrTexelPos = TexelPos[TexelId];
                                if (CurrTexelPos.x >= 0 && CurrTexelPos.x < Texture.Width &&
                                    CurrTexelPos.y >= 0 && CurrTexelPos.y < Texture.Height)
                                {
                                    TexelColors[TexelId] = ColorU32ToRgb(Texture.Texels[CurrTexelPos.y * Texture.Width + CurrTexelPos.x]);
                                }
                                else
                                {
                                    TexelColors[TexelId] = ColorU32ToRgb(Sampler.BorderColor);
                                }
                            }

                            f32 S = TexelV2.x - floorf(TexelV2.x);
                            f32 K = TexelV2.y - floorf(TexelV2.y);

                            v3 Interpolated0 = Lerp(TexelColors[0], TexelColors[1], S);
                            v3 Interpolated1 = Lerp(TexelColors[2], TexelColors[3], S);
                            v3 FinalColor = Lerp(Interpolated0, Interpolated1, K);

                            TexelColor = ColorRgbToU32(FinalColor);
                        } break;
                        
                        default:
                        {
                            InvalidCodePath;
                        }
                    }
                    
                    GlobalState.FrameBufferPixels[PixelId] = TexelColor;
                    GlobalState.DepthBuffer[PixelId] = DepthZ;
                }
            }

            Edge0 += Edge0DiffX;
            Edge1 += Edge1DiffX;
            Edge2 += Edge2DiffX;
        }

        Edge0RowY += Edge0DiffY;
        Edge1RowY += Edge1DiffY;
        Edge2RowY += Edge2DiffY;
    }
}

void DrawTriangle(v4 ModelVertex0, v4 ModelVertex1, v4 ModelVertex2,
                  v2 ModelUv0, v2 ModelUv1, v2 ModelUv2,
                  texture Texture, sampler Sampler)
{
    clip_result Ping = {};
    Ping.NumTriangles = 1;
    Ping.Vertices[0] = { ModelVertex0, ModelUv0 };
    Ping.Vertices[1] = { ModelVertex1, ModelUv1 };
    Ping.Vertices[2] = { ModelVertex2, ModelUv2 };
    
    clip_result Pong = {};
    
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Left);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Right);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Top);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Bottom);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Near);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Far);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_W);

    for (u32 TriangleId = 0; TriangleId < Pong.NumTriangles; ++TriangleId)
    {
        DrawTriangle(Pong.Vertices[3*TriangleId + 0], Pong.Vertices[3*TriangleId + 1],
                     Pong.Vertices[3*TriangleId + 2], Texture, Sampler);
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
        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        GlobalState.FrameBufferWidth = ClientRect.right - ClientRect.left;
        GlobalState.FrameBufferHeight = ClientRect.bottom - ClientRect.top;
            
        GlobalState.FrameBufferWidth = 1200;
        GlobalState.FrameBufferHeight = 1200;
        GlobalState.FrameBufferPixels = (u32*)malloc(sizeof(u32) * GlobalState.FrameBufferWidth *
                                                     GlobalState.FrameBufferHeight);
        GlobalState.DepthBuffer = (f32*)malloc(sizeof(f32) * GlobalState.FrameBufferWidth *
                                                     GlobalState.FrameBufferHeight);
    }

    texture CheckerBoardTexture = {};
    sampler Sampler = {};
    {
        Sampler.Type = SamplerType_Bilinear;
        Sampler.BorderColor = 0xFF000000;

        u32 BlockSize = 4;
        u32 NumBlocks = 8;
        
        CheckerBoardTexture.Width = BlockSize * NumBlocks;
        CheckerBoardTexture.Height = BlockSize * NumBlocks;
        CheckerBoardTexture.Texels = (u32*)malloc(sizeof(u32) * CheckerBoardTexture.Width * CheckerBoardTexture.Height);
        for (u32 Y = 0; Y < NumBlocks; ++Y)
        {
            for (u32 X = 0; X < NumBlocks; ++X)
            {
                u32 ColorChannel = 255 * ((X + (Y % 2)) % 2);

                for (u32 BlockY = 0; BlockY < BlockSize; ++BlockY)
                {
                    for (u32 BlockX = 0; BlockX < BlockSize; ++BlockX)
                    {
                        u32 TexelId = (Y * BlockSize + BlockY) * CheckerBoardTexture.Width + (X * BlockSize + BlockX);
                        CheckerBoardTexture.Texels[TexelId] = ((u32)0xFF << 24) | ((u32)ColorChannel << 16) | ((u32)ColorChannel << 8) | (u32)ColorChannel;
                    }
                }
            }
        }
    }
    
    LARGE_INTEGER BeginTime = {};
    LARGE_INTEGER EndTime = {};
    Assert(QueryPerformanceCounter(&BeginTime));
    
    while (GlobalState.IsRunning)
    {
        Assert(QueryPerformanceCounter(&EndTime));
        f32 FrameTime = f32(EndTime.QuadPart - BeginTime.QuadPart) / f32(TimerFrequency.QuadPart);
        BeginTime = EndTime;

        {
            char Text[256];
            snprintf(Text, sizeof(Text), "FrameTime: %f\n", FrameTime);
            OutputDebugStringA(Text);
        }
        
        MSG Message = {};
        while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
                case WM_QUIT:
                {
                    GlobalState.IsRunning = false;
                } break;

                case WM_KEYUP:
                case WM_KEYDOWN:
                {
                    u32 VkCode = Message.wParam;
                    b32 IsDown = !((Message.lParam >> 31) & 0x1);

                    switch (VkCode)
                    {
                        case 'W':
                        {
                            GlobalState.WDown = IsDown;
                        } break;

                        case 'A':
                        {
                            GlobalState.ADown = IsDown;
                        } break;

                        case 'S':
                        {
                            GlobalState.SDown = IsDown;
                        } break;

                        case 'D':
                        {
                            GlobalState.DDown = IsDown;
                        } break;
                    } 
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

                u8 Red = (u8)0;
                u8 Green = (u8)0;
                u8 Blue = 0;
                u8 Alpha = 255;
                u32 PixelColor = ((u32)Alpha << 24) | ((u32)Red << 16) | ((u32)Green << 8) | (u32)Blue;

                GlobalState.DepthBuffer[PixelId] = FLT_MAX;
                GlobalState.FrameBufferPixels[PixelId] = PixelColor;
            }
        }
        
        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        u32 ClientWidth = ClientRect.right - ClientRect.left;
        u32 ClientHeight = ClientRect.bottom - ClientRect.top;
        f32 AspectRatio = f32(ClientWidth) / f32(ClientHeight);
        
        // NOTE: Обчислюємо положення камери
        m4 CameraTransform = IdentityM4();
        {
            camera* Camera = &GlobalState.Camera;

            b32 MouseDown = false;
            v2 CurrMousePos = {};
            if (GetActiveWindow() == GlobalState.WindowHandle)
            {
                POINT Win32MousePos = {}; 
                Assert(GetCursorPos(&Win32MousePos));
                Assert(ScreenToClient(GlobalState.WindowHandle, &Win32MousePos));

                Win32MousePos.y = ClientRect.bottom - Win32MousePos.y;

                CurrMousePos.x = f32(Win32MousePos.x) / f32(ClientWidth);
                CurrMousePos.y = f32(Win32MousePos.y) / f32(ClientHeight);

                MouseDown = (GetKeyState(VK_LBUTTON) & 0x80) != 0;
            }

            if (MouseDown)
            {
                if (!Camera->PrevMouseDown)
                {
                    Camera->PrevMousePos = CurrMousePos;
                }

                v2 MouseDelta = CurrMousePos - Camera->PrevMousePos;
                Camera->Pitch += MouseDelta.y;
                Camera->Yaw += MouseDelta.x;

                Camera->PrevMousePos = CurrMousePos;
            }

            Camera->PrevMouseDown = MouseDown;
            
            m4 YawTransform = RotationMatrix(0, Camera->Yaw, 0);
            m4 PitchTransform = RotationMatrix(Camera->Pitch, 0, 0);
            m4 CameraAxisTransform = YawTransform * PitchTransform;

            v3 Right = Normalize((CameraAxisTransform * V4(1, 0, 0, 0)).xyz);
            v3 Up = Normalize((CameraAxisTransform * V4(0, 1, 0, 0)).xyz);
            v3 LookAt = Normalize((CameraAxisTransform * V4(0, 0, 1, 0)).xyz);

            m4 CameraViewTransform = IdentityM4();

            CameraViewTransform.v[0].x = Right.x;
            CameraViewTransform.v[1].x = Right.y;
            CameraViewTransform.v[2].x = Right.z;

            CameraViewTransform.v[0].y = Up.x;
            CameraViewTransform.v[1].y = Up.y;
            CameraViewTransform.v[2].y = Up.z;

            CameraViewTransform.v[0].z = LookAt.x;
            CameraViewTransform.v[1].z = LookAt.y;
            CameraViewTransform.v[2].z = LookAt.z;
            
            if (GlobalState.WDown)
            {
                Camera->Pos += LookAt * FrameTime;
            }
            if (GlobalState.SDown)
            {
                Camera->Pos -= LookAt * FrameTime;
            }
            if (GlobalState.DDown)
            {
                Camera->Pos += Right * FrameTime;
            }
            if (GlobalState.ADown)
            {
                Camera->Pos -= Right * FrameTime;
            }

            CameraTransform = CameraViewTransform * TranslationMatrix(-Camera->Pos);
        }
        
        // NOTE: Проєктуємо наші трикутники
        GlobalState.CurrTime = GlobalState.CurrTime + FrameTime;
        if (GlobalState.CurrTime > 2.0f * 3.14159f)
        {
            GlobalState.CurrTime -= 2.0f * 3.14159f;
        }

        GlobalState.CurrTime = 0;
        
        v3 ModelVertices[] =
        {
            // NOTE: Front Face
            V3(-0.5f, -0.5f, -0.5f),
            V3(-0.5f, 0.5f, -0.5f),
            V3(0.5f, 0.5f, -0.5f),
            V3(0.5f, -0.5f, -0.5f),

            // NOTE: Back Face
            V3(-0.5f, -0.5f, 0.5f),
            V3(-0.5f, 0.5f, 0.5f),
            V3(0.5f, 0.5f, 0.5f),
            V3(0.5f, -0.5f, 0.5f),
        };
        
        v2 ModelUvs[] =
        {
            V2(0, 0),
            V2(1, 0),
            V2(1, 1),
            V2(0, 1),

            V2(0, 0),
            V2(1, 0),
            V2(1, 1),
            V2(0, 1),
        };

        u32 ModelIndices[] =
        {
            // NOTE: Front Face
            0, 1, 2,
            2, 3, 0,

            // NOTE: Back Face
            6, 5, 4,
            4, 7, 6,

            // NOTE: Left face
            4, 5, 1,
            1, 0, 4,

            // NOTE: Right face
            3, 2, 6,
            6, 7, 3,

            // NOTE: Top face
            1, 5, 6,
            6, 2, 1,

            // NOTE: Bottom face
            4, 0, 3,
            3, 7, 4,
        };
        
        f32 Offset = abs(sin(GlobalState.CurrTime));
        m4 Transform = (PerspectiveMatrix(60.0f, AspectRatio, 0.01f, 1000.0f) *
                        CameraTransform *
                        TranslationMatrix(0, 0, 2) *
                        RotationMatrix(GlobalState.CurrTime, GlobalState.CurrTime, GlobalState.CurrTime) *
                        ScaleMatrix(1, 1, 1));

        v4* TransformedVertices = (v4*)malloc(sizeof(v4) * ArrayCount(ModelVertices));
        for (u32 VertexId = 0; VertexId < ArrayCount(ModelVertices); ++VertexId)
        {
            TransformedVertices[VertexId] = (Transform * V4(ModelVertices[VertexId], 1.0f));
        }
        
        for (u32 IndexId = 0; IndexId < ArrayCount(ModelIndices); IndexId += 3)
        {
            u32 Index0 = ModelIndices[IndexId + 0];
            u32 Index1 = ModelIndices[IndexId + 1];
            u32 Index2 = ModelIndices[IndexId + 2];
            
            DrawTriangle(TransformedVertices[Index0], TransformedVertices[Index1], TransformedVertices[Index2],
                         ModelUvs[Index0], ModelUvs[Index1], ModelUvs[Index2],
                         CheckerBoardTexture, Sampler);
        }

        free(TransformedVertices);
        
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
