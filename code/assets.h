#if !defined(ASSETS_H)

#undef global
#undef local_global

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define global static
#define local_global static

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct vertex
{
    v3 Pos;
    v2 Uv;
};

struct texture
{
    i32 Width;
    i32 Height;
    u32* Texels;

    ID3D12Resource* GpuTexture;
};

struct mesh
{
    u32 IndexOffset;
    u32 IndexCount;
    u32 VertexOffset;
    u32 VertexCount;
    u32 TextureId;
};

struct model
{
    u32 NumMeshes;
    mesh* MeshArray;

    u32 NumTextures;
    texture* TextureArray;
    
    u32 VertexCount;
    vertex* VertexArray;
    u32 IndexCount;
    u32* IndexArray;

    ID3D12Resource* GpuVertexBuffer;
    ID3D12Resource* GpuIndexBuffer;
};

#define ASSETS_H
#endif
