#if !defined(DX12_RASTERIZER_H)

#include <d3d12.h>
#include <dxgi1_3.h>

struct light_buffer_cpu
{
    v3 LightColor;
    f32 LightAmbientIntensity;
    v3 LightDirection;
    u32 Pad0;
    v3 CameraPos;
};

struct transform_buffer_cpu
{
    m4 WVPTransform;
    m4 WTransform;
    m4 NormalWTransform;
    f32 Shininess;
    f32 SpecularStrength;
};

struct dx12_arena
{
    u64 Size;
    u64 Used;
    ID3D12Heap* Heap;
};

struct dx12_upload_arena
{
    u64 Size;
    u64 Used;
    ID3D12Resource* GpuBuffer;
    u8* CpuPtr;
};

struct dx12_descriptor_heap
{
    ID3D12DescriptorHeap* Heap;
    u64 StepSize;
    u64 MaxNumElements;
    u64 CurrElement;
};

struct dx12_rasterizer
{
    IDXGIAdapter1* Adapter;
    ID3D12Device* Device;
    ID3D12CommandQueue* CommandQueue;

    dx12_upload_arena UploadArena;
    dx12_arena RtvArena;
    dx12_arena BufferArena;
    dx12_arena TextureArena;

    u32 RenderWidth;
    u32 RenderHeight;
    IDXGISwapChain1* SwapChain;
    u32 CurrentFrameIndex;
    ID3D12Resource* FrameBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE FrameBufferDescriptors[2];

    D3D12_CPU_DESCRIPTOR_HANDLE DepthDescriptor;
    ID3D12Resource* DepthBuffer;
    
    ID3D12CommandAllocator* CommandAllocator;

    ID3D12GraphicsCommandList* CommandList;
    ID3D12Fence* Fence;
    UINT64 FenceValue;

    dx12_descriptor_heap RtvHeap;
    dx12_descriptor_heap DsvHeap;
    dx12_descriptor_heap ShaderDescHeap;

    ID3D12Resource* TransformBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE TransformDescriptor;

    ID3D12Resource* LightBuffer;
    
    // NOTE: Графічний конвеєр
    ID3D12RootSignature* ModelRootSignature;
    ID3D12PipelineState* ModelPipeline;
};

#define DX12_RASTERIZER_H
#endif
