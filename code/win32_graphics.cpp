
#include <cmath>
#include <windows.h>

#include "win32_graphics.h"
#include "graphics_math.cpp"
#include "clipper.cpp"
#include "dx12_rasterizer.cpp"
#include "assets.cpp"
#include "sw_rasterizer.cpp"

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
    }

    GlobalState.RasterizerType = RasterizerType_Dx12;
    GlobalState.SwRasterizer = SwRasterizerCreate(GlobalState.WindowHandle, 600, 600);
    GlobalState.Dx12Rasterizer = Dx12RasterizerCreate(GlobalState.WindowHandle, 1920, 1080);
    
    {
        local_global vertex ModelVertices[] =
        {
            // NOTE: Front Face
            { V3(-0.5f, -0.5f, -0.5f), V2(0, 0) },
            { V3(-0.5f, 0.5f, -0.5f), V2(1, 0) },
            { V3(0.5f, 0.5f, -0.5f), V2(1, 1) },
            { V3(0.5f, -0.5f, -0.5f), V2(0, 1) },

            // NOTE: Back Face
            { V3(-0.5f, -0.5f, 0.5f), V2(0, 0) },
            { V3(-0.5f, 0.5f, 0.5f), V2(1, 0) },
            { V3(0.5f, 0.5f, 0.5f), V2(1, 1) }, 
            { V3(0.5f, -0.5f, 0.5f), V2(0, 1) },
        };

        local_global u32 ModelIndices[] =
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
                
        local_global mesh Mesh = {};
        Mesh.IndexOffset = 0;
        Mesh.IndexCount = ArrayCount(ModelIndices);
        Mesh.VertexOffset = 0;
        Mesh.VertexCount = ArrayCount(ModelVertices);
        
        GlobalState.CubeModel = {};
        GlobalState.CubeModel.NumMeshes = 1;
        GlobalState.CubeModel.MeshArray = &Mesh;

        GlobalState.CubeModel.VertexCount = ArrayCount(ModelVertices);
        GlobalState.CubeModel.VertexArray = ModelVertices;
        GlobalState.CubeModel.IndexCount = ArrayCount(ModelIndices);
        GlobalState.CubeModel.IndexArray = ModelIndices;
    }

    GlobalState.DuckModel = AssetLoadModel(&GlobalState.Dx12Rasterizer, "Duck\\", "Duck.gltf");
    GlobalState.SponzaModel = AssetLoadModel(&GlobalState.Dx12Rasterizer, "Sponza\\", "Sponza.gltf");
    
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

    switch (GlobalState.RasterizerType)
    {
        case RasterizerType_Dx12:
        {
            dx12_rasterizer* Rasterizer = &GlobalState.Dx12Rasterizer;
            ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;
            
            ThrowIfFailed(CommandList->Close());
            Rasterizer->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);

            Rasterizer->FenceValue += 1;
            ThrowIfFailed(Rasterizer->CommandQueue->Signal(Rasterizer->Fence, Rasterizer->FenceValue));
            if (Rasterizer->Fence->GetCompletedValue() != Rasterizer->FenceValue)
            {
                // NOTE: Чекаємо аж поки значення у паркані не дорівнює FenceValue
                HANDLE FenceEvent = {};
                ThrowIfFailed(Rasterizer->Fence->SetEventOnCompletion(Rasterizer->FenceValue, FenceEvent));
                WaitForSingleObject(FenceEvent, INFINITE);
            }

            ThrowIfFailed(Rasterizer->CommandAllocator->Reset());
            ThrowIfFailed(CommandList->Reset(Rasterizer->CommandAllocator, 0));

            Dx12ClearUploadArena(&Rasterizer->UploadArena);
        } break;
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
                        
        f32 Offset = abs(sin(GlobalState.CurrTime));
        m4 Transform = (PerspectiveMatrix(60.0f, AspectRatio, 0.01f, 1000.0f) *
                        CameraTransform *
                        TranslationMatrix(0, 0, 1) *
                        RotationMatrix(0, Pi32 * 0.5f, 0) *
                        ScaleMatrix(1, 1, 1));

        switch (GlobalState.RasterizerType)
        {
            case RasterizerType_Software:
            {
                sw_rasterizer* Rasterizer = &GlobalState.SwRasterizer;
                        
                // NOTE: Очистуємо буфер кадри до 0
                for (u32 Y = 0; Y < Rasterizer->FrameBufferHeight; ++Y)
                {
                    for (u32 X = 0; X < Rasterizer->FrameBufferWidth; ++X)
                    {
                        u32 PixelId = Y * Rasterizer->FrameBufferStride + X;

                        u8 Red = (u8)0;
                        u8 Green = (u8)0;
                        u8 Blue = 0;
                        u8 Alpha = 255;
                        u32 PixelColor = ((u32)Alpha << 24) | ((u32)Red << 16) | ((u32)Green << 8) | (u32)Blue;
                        
                        Rasterizer->DepthBuffer[PixelId] = FLT_MAX;
                        Rasterizer->FrameBufferPixels[PixelId] = PixelColor;
                    }
                }

                DrawModel(Rasterizer, &GlobalState.SponzaModel, Transform, Sampler);
        
                BITMAPINFO BitmapInfo = {};
                BitmapInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
                BitmapInfo.bmiHeader.biWidth = Rasterizer->FrameBufferStride;
                BitmapInfo.bmiHeader.biHeight = Rasterizer->FrameBufferHeight;
                BitmapInfo.bmiHeader.biPlanes = 1;
                BitmapInfo.bmiHeader.biBitCount = 32;
                BitmapInfo.bmiHeader.biCompression = BI_RGB;
        
                Assert(StretchDIBits(Rasterizer->DeviceContext,
                                     0,
                                     0,
                                     ClientWidth,
                                     ClientHeight,
                                     0,
                                     0,
                                     Rasterizer->FrameBufferWidth,
                                     Rasterizer->FrameBufferHeight,
                                     Rasterizer->FrameBufferPixels,
                                     &BitmapInfo,
                                     DIB_RGB_COLORS,
                                     SRCCOPY));
            } break;

            case RasterizerType_Dx12:
            {
                dx12_rasterizer* Rasterizer = &GlobalState.Dx12Rasterizer;
                ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

                Dx12CopyDataToBuffer(Rasterizer,
                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                     &Transform,
                                     sizeof(m4),
                                     Rasterizer->TransformBuffer);
                
                {
                    D3D12_RESOURCE_BARRIER Barrier = {};
                    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    Barrier.Transition.pResource = Rasterizer->FrameBuffers[Rasterizer->CurrentFrameIndex];
                    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    CommandList->ResourceBarrier(1, &Barrier);
                }
                
                const FLOAT Color[4] = { 1, 0, 1, 1 };
                CommandList->ClearRenderTargetView(Rasterizer->FrameBufferDescriptors[Rasterizer->CurrentFrameIndex], Color, 0, nullptr);
                CommandList->ClearDepthStencilView(Rasterizer->DepthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, 0);

                CommandList->OMSetRenderTargets(1, Rasterizer->FrameBufferDescriptors + Rasterizer->CurrentFrameIndex, 0,
                                                &Rasterizer->DepthDescriptor);

                D3D12_VIEWPORT Viewport = {};
                Viewport.TopLeftX = 0;
                Viewport.TopLeftY = 0;
                Viewport.Width = Rasterizer->RenderWidth;
                Viewport.Height = Rasterizer->RenderHeight;
                Viewport.MinDepth = 0;
                Viewport.MaxDepth = 1;
                CommandList->RSSetViewports(1, &Viewport);

                D3D12_RECT ScissorRect = {};
                ScissorRect.left = 0;
                ScissorRect.right = Rasterizer->RenderWidth;
                ScissorRect.top = 0;
                ScissorRect.bottom = Rasterizer->RenderHeight;
                CommandList->RSSetScissorRects(1, &ScissorRect);

                model* CurrModel = &GlobalState.SponzaModel;
                {
                    D3D12_INDEX_BUFFER_VIEW View = {};
                    View.BufferLocation = CurrModel->GpuIndexBuffer->GetGPUVirtualAddress();
                    View.SizeInBytes = sizeof(u32) * CurrModel->IndexCount;
                    View.Format = DXGI_FORMAT_R32_UINT;
                    CommandList->IASetIndexBuffer(&View);
                }

                {
                    D3D12_VERTEX_BUFFER_VIEW Views[1] = {};
                    Views[0].BufferLocation = CurrModel->GpuVertexBuffer->GetGPUVirtualAddress();
                    Views[0].SizeInBytes = sizeof(vertex) * CurrModel->VertexCount;
                    Views[0].StrideInBytes = sizeof(vertex);
                    CommandList->IASetVertexBuffers(0, 1, Views);
                }

                CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                CommandList->SetGraphicsRootSignature(Rasterizer->ModelRootSignature);
                CommandList->SetPipelineState(Rasterizer->ModelPipeline);

                CommandList->SetDescriptorHeaps(1, &Rasterizer->ShaderDescHeap.Heap);
                CommandList->SetGraphicsRootDescriptorTable(1, Rasterizer->TransformDescriptor);

                for (u32 MeshId = 0; MeshId < CurrModel->NumMeshes; ++MeshId)
                {
                    mesh* CurrMesh = CurrModel->MeshArray + MeshId;
                    texture* CurrTexture = CurrModel->TextureArray + CurrMesh->TextureId;
                        
                    CommandList->SetGraphicsRootDescriptorTable(0, CurrTexture->GpuDescriptor);
                    CommandList->DrawIndexedInstanced(CurrMesh->IndexCount, 1, CurrMesh->IndexOffset, CurrMesh->VertexOffset, 0);
                }
                
                {
                    D3D12_RESOURCE_BARRIER Barrier = {};
                    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    Barrier.Transition.pResource = Rasterizer->FrameBuffers[Rasterizer->CurrentFrameIndex];
                    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                    CommandList->ResourceBarrier(1, &Barrier);
                }
                
                ThrowIfFailed(CommandList->Close());
                Rasterizer->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CommandList);
                Rasterizer->SwapChain->Present(1, 0);

                Rasterizer->FenceValue += 1;
                ThrowIfFailed(Rasterizer->CommandQueue->Signal(Rasterizer->Fence, Rasterizer->FenceValue));
                if (Rasterizer->Fence->GetCompletedValue() != Rasterizer->FenceValue)
                {
                    // NOTE: Чекаємо аж поки значення у паркані не дорівнює FenceValue
                    HANDLE FenceEvent = {};
                    ThrowIfFailed(Rasterizer->Fence->SetEventOnCompletion(Rasterizer->FenceValue, FenceEvent));
                    WaitForSingleObject(FenceEvent, INFINITE);
                }

                ThrowIfFailed(Rasterizer->CommandAllocator->Reset());
                ThrowIfFailed(CommandList->Reset(Rasterizer->CommandAllocator, 0));

                Rasterizer->CurrentFrameIndex = (Rasterizer->CurrentFrameIndex + 1) % 2;
                Dx12ClearUploadArena(&Rasterizer->UploadArena);
            } break;
        }
    }
    
    return 0;
}
