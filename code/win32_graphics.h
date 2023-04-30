#if !defined(WIN32_GRAPHICS_H)

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t mm;
typedef uintptr_t umm;

typedef int32_t b32;

#define global static
#define local_global static

#define snprintf _snprintf_s
#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define InvalidCodePath Assert(!"Invalid Code Path")
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)
#define TeraBytes(Val) (GigaBytes(Val)*1024LL)

#include "graphics_math.h"
#include "clipper.h"

struct texture
{
    i32 Width;
    i32 Height;
    u32* Texels;
};

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

struct camera
{
    b32 PrevMouseDown;
    v2 PrevMousePos;

    f32 Yaw;
    f32 Pitch;
    
    v3 Pos;
};

struct global_state
{
    b32 IsRunning;
    HWND WindowHandle;
    HDC DeviceContext;
    u32 FrameBufferWidth;
    u32 FrameBufferHeight;
    u32 FrameBufferStride;
    u32* FrameBufferPixels;
    f32* DepthBuffer;
    
    f32 CurrTime;

    b32 WDown;
    b32 ADown;
    b32 SDown;
    b32 DDown;
    camera Camera;
};

#define WIN32_GRAPHICS_H
#endif
