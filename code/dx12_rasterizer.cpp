
void ThrowIfFailed(HRESULT Result)
{
    if (Result != S_OK)
    {
        InvalidCodePath;
    }
}

dx12_rasterizer Dx12RasterizerCreate(HWND WindowHandle, u32 Width, u32 Height)
{
    dx12_rasterizer Result = {};

    IDXGIFactory2* Factory = 0;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)));

    ID3D12Debug1* Debug;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
    Debug->EnableDebugLayer();
    Debug->SetEnableGPUBasedValidation(true);
    
    for (u32 AdapterIndex = 0;
         Factory->EnumAdapters1(AdapterIndex, &Result.Adapter) != DXGI_ERROR_NOT_FOUND;
         ++AdapterIndex)
    {
        DXGI_ADAPTER_DESC1 Desc;
        Result.Adapter->GetDesc1(&Desc);

        if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(Result.Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Result.Device))))
        {
            break;
        }
    }

    ID3D12Device* Device = Result.Device;
    
    {
        D3D12_COMMAND_QUEUE_DESC Desc = {};
        Desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(Device->CreateCommandQueue(&Desc, IID_PPV_ARGS(&Result.CommandQueue)));
    }

    {
        DXGI_SWAP_CHAIN_DESC1 Desc = {};
        Desc.Width = Width;
        Desc.Height = Height;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 1;
        Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Desc.BufferCount = 2;
        Desc.Scaling = DXGI_SCALING_STRETCH;
        Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        ThrowIfFailed(Factory->CreateSwapChainForHwnd(Result.CommandQueue, WindowHandle, &Desc, nullptr, nullptr, &Result.SwapChain));

        Result.CurrentFrameIndex = 0;
        ThrowIfFailed(Result.SwapChain->GetBuffer(0, IID_PPV_ARGS(&Result.FrameBuffers[0])));
        ThrowIfFailed(Result.SwapChain->GetBuffer(1, IID_PPV_ARGS(&Result.FrameBuffers[1])));
    }

    ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Result.CommandAllocator)));
    ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Result.CommandAllocator, 0, IID_PPV_ARGS(&Result.CommandList)));
    ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Result.Fence)));
    Result.FenceValue = 0;
            
    {
        D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
        Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        Desc.NumDescriptors = 2;
        Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Result.RtvHeap)));

        u32 DescriptorStepSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        
        Result.FrameBufferDescriptors[0] = Result.RtvHeap->GetCPUDescriptorHandleForHeapStart();
        Device->CreateRenderTargetView(Result.FrameBuffers[0], nullptr, Result.FrameBufferDescriptors[0]);
        Result.FrameBufferDescriptors[1].ptr = Result.FrameBufferDescriptors[0].ptr + DescriptorStepSize;
        Device->CreateRenderTargetView(Result.FrameBuffers[1], nullptr, Result.FrameBufferDescriptors[1]);
    }
    
    return Result;
}
