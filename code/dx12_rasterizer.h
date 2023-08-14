#if !defined(DX12_RASTERIZER_H)

#include <d3d12.h>
#include <dxgi1_3.h>

struct dx12_rasterizer
{
    IDXGIAdapter1* Adapter;
    ID3D12Device* Device;
    ID3D12CommandQueue* CommandQueue;

    IDXGISwapChain1* SwapChain;
    u32 CurrentFrameIndex;
    ID3D12Resource* FrameBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE FrameBufferDescriptors[2];

    ID3D12CommandAllocator* CommandAllocator;

    ID3D12GraphicsCommandList* CommandList;
    ID3D12Fence* Fence;
    UINT64 FenceValue;

    ID3D12DescriptorHeap* RtvHeap;
};

#define DX12_RASTERIZER_H
#endif
