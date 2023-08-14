#if !defined(SW_RASTERIZER_H)

enum sampler_type
{
    SamplerType_None,

    SamplerType_Nearest,
    SamplerType_Bilinear,
};

struct sampler
{
    sampler_type Type;
    u32 BorderColor;
};

struct sw_rasterizer
{
    HDC DeviceContext;
    u32 FrameBufferWidth;
    u32 FrameBufferHeight;
    u32 FrameBufferStride;
    u32* FrameBufferPixels;
    f32* DepthBuffer;
};

#define SW_RASTERIZER_H
#endif
