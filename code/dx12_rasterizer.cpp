
void ThrowIfFailed(HRESULT Result)
{
    if (Result != S_OK)
    {
        InvalidCodePath;
    }
}

u32 Dx12GetBytesPerPixel(DXGI_FORMAT Format)
{
    u32 Result = 0;
    
    switch (Format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            Result = 16;
        } break;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        {
            Result = 8;
        } break;
        
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        {
            Result = 4;
        } break;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        {
            Result = 2;
        } break;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        {
            Result = 1;
        } break;
    }

    return Result;
}

//
// NOTE: Дескрипторна Купа
//

dx12_descriptor_heap Dx12DescriptorHeapCreate(dx12_rasterizer* Rasterizer,
                                              D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                              u64 NumDescriptors,
                                              D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
{
    dx12_descriptor_heap Result = {};
    ID3D12Device* Device = Rasterizer->Device;

    Result.MaxNumElements = NumDescriptors;
    Result.StepSize = Device->GetDescriptorHandleIncrementSize(Type);

    D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
    Desc.Type = Type;
    Desc.NumDescriptors = NumDescriptors;
    Desc.Flags = Flags;
    ThrowIfFailed(Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Result.Heap)));
    
    return Result;
}

void Dx12DescriptorAllocate(dx12_descriptor_heap* Heap,
                            D3D12_CPU_DESCRIPTOR_HANDLE* OutCpuHandle,
                            D3D12_GPU_DESCRIPTOR_HANDLE* OutGpuHandle)
{
    Assert(Heap->CurrElement < Heap->MaxNumElements);
    
    if (OutCpuHandle)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = Heap->Heap->GetCPUDescriptorHandleForHeapStart();
        CpuHandle.ptr += Heap->StepSize * Heap->CurrElement;
        *OutCpuHandle = CpuHandle;
    }

    if (OutGpuHandle)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = Heap->Heap->GetGPUDescriptorHandleForHeapStart();
        GpuHandle.ptr += Heap->StepSize * Heap->CurrElement;
        *OutGpuHandle = GpuHandle;
    }

    Heap->CurrElement += 1;
}

//
// NOTE: Лінійна Арена
//

dx12_arena Dx12ArenaCreate(ID3D12Device* Device, D3D12_HEAP_TYPE Type, u64 Size,
                           D3D12_HEAP_FLAGS Flags)
{
    dx12_arena Result = {};
    Result.Size = Size;

    D3D12_HEAP_DESC Desc = {};
    Desc.SizeInBytes = Size;
    Desc.Properties.Type = Type;
    Desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    Desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    Desc.Flags = Flags;

    ThrowIfFailed(Device->CreateHeap(&Desc, IID_PPV_ARGS(&Result.Heap)));
    
    return Result;
}

ID3D12Resource* Dx12CreateResource(dx12_rasterizer* Rasterizer, dx12_arena* Arena,
                                   D3D12_RESOURCE_DESC* Desc,
                                   D3D12_RESOURCE_STATES InitialState,
                                   D3D12_CLEAR_VALUE* ClearValues)
{
    ID3D12Resource* Result = 0;
    ID3D12Device* Device = Rasterizer->Device;

    D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Device->GetResourceAllocationInfo(0, 1, Desc);
    u64 GpuAlignedOffset = Align(Arena->Used, AllocationInfo.Alignment);
    Assert((GpuAlignedOffset + AllocationInfo.SizeInBytes) < Arena->Size);
    
    ThrowIfFailed(Device->CreatePlacedResource(Arena->Heap, GpuAlignedOffset,
                                               Desc, InitialState, ClearValues,
                                               IID_PPV_ARGS(&Result)));
    Arena->Used = GpuAlignedOffset + AllocationInfo.SizeInBytes;

    return Result;
}

//
// NOTE: Арена Завантаження
//

dx12_upload_arena Dx12UploadArenaCreate(dx12_rasterizer* Rasterizer, u64 Size)
{
    dx12_upload_arena Result = {};
    Result.Size = Size;

    ID3D12Device* Device = Rasterizer->Device;
            
    D3D12_HEAP_PROPERTIES HeapProperties = {};
    HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Size;
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ThrowIfFailed(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &Desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  0, IID_PPV_ARGS(&Result.GpuBuffer)));
    Result.GpuBuffer->Map(0, 0, (void**)&Result.CpuPtr);

    return Result;
}

u8* Dx12UploadArenaPushSize(dx12_upload_arena* Arena, u64 Size, u64* OutOffset)
{
    u64 AlignedOffset = Align(Arena->Used, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    Assert((AlignedOffset + Size) < Arena->Size);
    
    u8* Result = Arena->CpuPtr + AlignedOffset;
    Arena->Used = AlignedOffset + Size;
    *OutOffset = AlignedOffset;
    
    return Result;
}

void Dx12ClearUploadArena(dx12_upload_arena* Arena)
{
    Arena->Used = 0;
}

//
// NOTE: Творення Ресурсів зі копiювання
//

void Dx12CopyDataToBuffer(dx12_rasterizer* Rasterizer,
                          D3D12_RESOURCE_STATES StartState,
                          D3D12_RESOURCE_STATES EndState,
                          void* Data,
                          u64 DataSize,
                          ID3D12Resource* GpuBuffer)
{
    ID3D12Device* Device = Rasterizer->Device;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    u64 UploadOffset = 0;
    {
        u8* Dest = Dx12UploadArenaPushSize(&Rasterizer->UploadArena, DataSize, &UploadOffset);
        memcpy(Dest, Data, DataSize);
    }

    // NOTE: Барєр щоби перетворити стан текстури
    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = GpuBuffer;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = StartState;
        Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        CommandList->ResourceBarrier(1, &Barrier);
    }

    CommandList->CopyBufferRegion(GpuBuffer, 0, Rasterizer->UploadArena.GpuBuffer, UploadOffset, DataSize);

    // NOTE: Барєр щоби перетворити стан текстури
    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = GpuBuffer;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        Barrier.Transition.StateAfter = EndState;
        CommandList->ResourceBarrier(1, &Barrier);
    }
}

ID3D12Resource* Dx12CreateBufferAsset(dx12_rasterizer* Rasterizer,
                                      D3D12_RESOURCE_DESC* Desc,
                                      D3D12_RESOURCE_STATES InitialState,
                                      void* BufferData)
{
    ID3D12Resource* Result = 0;

    ID3D12Device* Device = Rasterizer->Device;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    u64 UploadOffset = 0;
    {
        u8* Dest = Dx12UploadArenaPushSize(&Rasterizer->UploadArena,
                                           Desc->Width, &UploadOffset);
        memcpy(Dest, BufferData, Desc->Width);
    }

    Result = Dx12CreateResource(Rasterizer, &Rasterizer->BufferArena, Desc,
                                D3D12_RESOURCE_STATE_COPY_DEST, 0);
    CommandList->CopyBufferRegion(Result, 0,
                                  Rasterizer->UploadArena.GpuBuffer,
                                  UploadOffset, Desc->Width);

    // NOTE: Барєр щоби перетворити стан буферу
    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = Result;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        Barrier.Transition.StateAfter = InitialState;
        CommandList->ResourceBarrier(1, &Barrier);
    }
    
    return Result;
}

ID3D12Resource* Dx12CreateTextureAsset(dx12_rasterizer* Rasterizer,
                                       D3D12_RESOURCE_DESC* Desc,
                                       D3D12_RESOURCE_STATES InitialState,
                                       void* Texels)
{
    ID3D12Resource* Result = 0;

    ID3D12Device* Device = Rasterizer->Device;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Device->GetResourceAllocationInfo(0, 1, Desc);
    u64 BytesPerPixel = Dx12GetBytesPerPixel(Desc->Format);
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootPrint = {};
    {
        Assert(Desc->DepthOrArraySize == 1);
        D3D12_SUBRESOURCE_FOOTPRINT FootPrint = {};
        FootPrint.Format = Desc->Format;
        FootPrint.Width = Desc->Width;
        FootPrint.Height = Desc->Height;
        FootPrint.Depth = 1;
        FootPrint.RowPitch = Align(FootPrint.Width * BytesPerPixel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        PlacedFootPrint.Footprint = FootPrint;

        u8* DestTexels = Dx12UploadArenaPushSize(&Rasterizer->UploadArena,
                                                 FootPrint.Height * FootPrint.RowPitch,
                                                 &PlacedFootPrint.Offset);

        // NOTE: Копюємо текстуру до Upload Arena
        for (u32 Y = 0; Y < Desc->Height; ++Y)
        {
            u8* Src = (u8*)Texels + (Y * Desc->Width) * BytesPerPixel;
            u8* Dest = DestTexels + (Y * FootPrint.RowPitch);
            memcpy(Dest, Src, BytesPerPixel * Desc->Width);
        }
    }

    // NOTE: Творимо текстуру
    Result = Dx12CreateResource(Rasterizer, &Rasterizer->TextureArena,
                                Desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);

    // NOTE: Копюємо текстуру
    {
        D3D12_TEXTURE_COPY_LOCATION SrcRegion = {};
        SrcRegion.pResource = Rasterizer->UploadArena.GpuBuffer;
        SrcRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        SrcRegion.PlacedFootprint = PlacedFootPrint;

        D3D12_TEXTURE_COPY_LOCATION DstRegion = {};
        DstRegion.pResource = Result;
        DstRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        DstRegion.SubresourceIndex = 0;

        CommandList->CopyTextureRegion(&DstRegion, 0, 0, 0, &SrcRegion, nullptr);
    }

    // NOTE: Барєр щоби перетворити стан текстури
    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = Result;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        Barrier.Transition.StateAfter = InitialState;
        CommandList->ResourceBarrier(1, &Barrier);
    }
    
    return Result;
}

//
// NOTE: Функції з шейдерами
//

D3D12_SHADER_BYTECODE Dx12LoadShader(char* FileName)
{
    D3D12_SHADER_BYTECODE Result = {};
    
    FILE* File = fopen(FileName, "rb");
    Assert(File);

    fseek(File, 0, SEEK_END);
    Result.BytecodeLength = ftell(File);
    fseek(File, 0, SEEK_SET);

    void* FileData = malloc(Result.BytecodeLength);
    fread(FileData, Result.BytecodeLength, 1, File);
    Result.pShaderBytecode = FileData;

    fclose(File);

    return Result;
}

dx12_rasterizer Dx12RasterizerCreate(HWND WindowHandle, u32 Width, u32 Height)
{
    dx12_rasterizer Result = {};
    Result.RenderWidth = Width;
    Result.RenderHeight = Height;

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

    Result.RtvArena = Dx12ArenaCreate(Device, D3D12_HEAP_TYPE_DEFAULT,
                                      MegaBytes(50),
                                      D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);

    Result.UploadArena = Dx12UploadArenaCreate(&Result, MegaBytes(300));
    
    Result.BufferArena = Dx12ArenaCreate(Device, D3D12_HEAP_TYPE_DEFAULT,
                                         MegaBytes(100),
                                         D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

    Result.TextureArena = Dx12ArenaCreate(Device, D3D12_HEAP_TYPE_DEFAULT,
                                          MegaBytes(300),
                                          D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);

    Result.RtvHeap = Dx12DescriptorHeapCreate(&Result, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
    Result.DsvHeap = Dx12DescriptorHeapCreate(&Result, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    Result.ShaderDescHeap = Dx12DescriptorHeapCreate(&Result, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 50, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    
    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        Desc.Width = Width;
        Desc.Height = Height;
        Desc.DepthOrArraySize = 1;
        Desc.MipLevels = 1;
        Desc.Format = DXGI_FORMAT_D32_FLOAT;
        Desc.SampleDesc.Count = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Desc.Format;
        ClearValue.DepthStencil.Depth = 1;
        
        Result.DepthBuffer = Dx12CreateResource(&Result, &Result.RtvArena, &Desc,
                                                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                &ClearValue);
        Dx12DescriptorAllocate(&Result.DsvHeap, &Result.DepthDescriptor, 0);
        Device->CreateDepthStencilView(Result.DepthBuffer, 0, Result.DepthDescriptor);
    }
    
    {
        Dx12DescriptorAllocate(&Result.RtvHeap, Result.FrameBufferDescriptors + 0, 0);
        Device->CreateRenderTargetView(Result.FrameBuffers[0], nullptr, Result.FrameBufferDescriptors[0]);
        Dx12DescriptorAllocate(&Result.RtvHeap, Result.FrameBufferDescriptors + 1, 0);
        Device->CreateRenderTargetView(Result.FrameBuffers[1], nullptr, Result.FrameBufferDescriptors[1]);
    }

    // NOTE: Творимо графічний конвеєр
    {
        // NOTE: Творимо графічний підпис
        {
            D3D12_ROOT_PARAMETER RootParameters[2] = {};

            // NOTE: Творимо першу таблицю дескрипторів
            D3D12_DESCRIPTOR_RANGE Table1Range[1] = {};
            {
                Table1Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                Table1Range[0].NumDescriptors = 1;
                Table1Range[0].BaseShaderRegister = 0;
                Table1Range[0].RegisterSpace = 0;
                Table1Range[0].OffsetInDescriptorsFromTableStart = 0;

                RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[0].DescriptorTable.NumDescriptorRanges = ArrayCount(Table1Range);
                RootParameters[0].DescriptorTable.pDescriptorRanges = Table1Range;
                RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }

            // NOTE: Творимо другу таблицю дескрипторів
            D3D12_DESCRIPTOR_RANGE Table2Range[1] = {};
            {
                Table2Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                Table2Range[0].NumDescriptors = 1;
                Table2Range[0].BaseShaderRegister = 0;
                Table2Range[0].RegisterSpace = 0;
                Table2Range[0].OffsetInDescriptorsFromTableStart = 0;

                RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[1].DescriptorTable.NumDescriptorRanges = ArrayCount(Table2Range);
                RootParameters[1].DescriptorTable.pDescriptorRanges = Table2Range;
                RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }

            D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc = {};
            StaticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            StaticSamplerDesc.ShaderRegister = 0;
            StaticSamplerDesc.RegisterSpace = 0;
            StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            
            D3D12_ROOT_SIGNATURE_DESC SignatureDesc = {};
            SignatureDesc.NumParameters = ArrayCount(RootParameters);
            SignatureDesc.pParameters = RootParameters;
            SignatureDesc.NumStaticSamplers = 1;
            SignatureDesc.pStaticSamplers = &StaticSamplerDesc;
            SignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            
            ID3DBlob* SerializedRootSig = 0;
            ID3DBlob* ErrorBlob = 0;
            ThrowIfFailed(D3D12SerializeRootSignature(&SignatureDesc,
                                                      D3D_ROOT_SIGNATURE_VERSION_1_0,
                                                      &SerializedRootSig,
                                                      &ErrorBlob));
            ThrowIfFailed(Device->CreateRootSignature(0,
                                                      SerializedRootSig->GetBufferPointer(),
                                                      SerializedRootSig->GetBufferSize(),
                                                      IID_PPV_ARGS(&Result.ModelRootSignature)));

            if (SerializedRootSig)
            {
                SerializedRootSig->Release();
            }
            if (ErrorBlob)
            {
                ErrorBlob->Release();
            }
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
        Desc.pRootSignature = Result.ModelRootSignature;
        
        Desc.VS = Dx12LoadShader("ModelVsMain.shader");
        Desc.PS = Dx12LoadShader("ModelPsMain.shader");

        Desc.BlendState.RenderTarget[0].BlendEnable = true;
        Desc.BlendState.RenderTarget[0].LogicOpEnable = false;
        Desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        Desc.SampleMask = 0xFFFFFFFF;

        Desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        Desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        Desc.RasterizerState.FrontCounterClockwise = FALSE;
        Desc.RasterizerState.DepthClipEnable = TRUE;

        Desc.DepthStencilState.DepthEnable = TRUE;
        Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        
        D3D12_INPUT_ELEMENT_DESC InputElementDescs[2] = {};
        InputElementDescs[0].SemanticName = "POSITION";
        InputElementDescs[0].SemanticIndex = 0;
        InputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        InputElementDescs[0].InputSlot = 0;
        InputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

        InputElementDescs[1].SemanticName = "TEXCOORD";
        InputElementDescs[1].SemanticIndex = 0;
        InputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
        InputElementDescs[1].InputSlot = 0;
        InputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        
        Desc.InputLayout.pInputElementDescs = InputElementDescs;
        Desc.InputLayout.NumElements = ArrayCount(InputElementDescs);

        Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        Desc.NumRenderTargets = 1;
        Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        Desc.SampleDesc.Count = 1;
        
        ThrowIfFailed(Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&Result.ModelPipeline)));
    }

    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        Desc.Width = Align(sizeof(m4), 256);
        Desc.Height = 1;
        Desc.DepthOrArraySize = 1;
        Desc.MipLevels = 1;
        Desc.Format = DXGI_FORMAT_UNKNOWN;
        Desc.SampleDesc.Count = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        Result.TransformBuffer = Dx12CreateResource(&Result,
                                                    &Result.BufferArena,
                                                    &Desc,
                                                    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                                    0);

        D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc = {};
        CbvDesc.BufferLocation = Result.TransformBuffer->GetGPUVirtualAddress();
        CbvDesc.SizeInBytes = Desc.Width;

        D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
        Dx12DescriptorAllocate(&Result.ShaderDescHeap, &CpuDescriptor, &Result.TransformDescriptor);
        Device->CreateConstantBufferView(&CbvDesc, CpuDescriptor);
    }
    
    return Result;
}
