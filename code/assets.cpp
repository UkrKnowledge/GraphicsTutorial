
char* CombineStrings(const char* Str1, const char* Str2)
{
    char* Result = 0;
    mm Length1 = strlen(Str1);
    mm Length2 = strlen(Str2);
    Result = (char*)malloc(sizeof(char) * (Length1 + Length2 + 1));
    memcpy(Result, Str1, Length1 * sizeof(char));
    memcpy(Result + Length1, Str2, Length2 * sizeof(char));
    Result[Length1 + Length2] = 0;

    return Result;
}

texture AssetCreateDefaultNormalMap()
{
    local_global u32 NormalTexel = 0xFFFF0000;
    
    texture Result = {};
    Result.Width = 1;
    Result.Height = 1;
    Result.Texels = &NormalTexel;
    
    return Result;
}

model AssetCreateCube(dx12_rasterizer* Dx12Rasterizer)
{
    model Result = {};

    {
        local_global u32 ColorTexel = 0xFFFFFFFF;

        Result.NumTextures = 1;
        Result.ColorTextureArray = (texture*)malloc(sizeof(texture));
        Result.NormalTextureArray = (texture*)malloc(sizeof(texture));
        
        // NOTE: Творимо дифузну текстуру
        {
            *Result.ColorTextureArray = {};
            Result.ColorTextureArray[0].Width = 1;
            Result.ColorTextureArray[0].Height = 1;
            Result.ColorTextureArray[0].Texels = &ColorTexel;
            
            Dx12CreateTexture(Dx12Rasterizer, Result.ColorTextureArray[0].Width, Result.ColorTextureArray[0].Height, (u8*)Result.ColorTextureArray[0].Texels,
                              &Result.ColorTextureArray[0].GpuTexture, &Result.ColorTextureArray[0].GpuDescriptor);
        }
        
        // NOTE: Творимо нормаль текстуру
        {
            Result.NormalTextureArray[0] = AssetCreateDefaultNormalMap();
            Dx12CreateTexture(Dx12Rasterizer, Result.NormalTextureArray[0].Width, Result.NormalTextureArray[0].Height, (u8*)Result.NormalTextureArray[0].Texels,
                              &Result.NormalTextureArray[0].GpuTexture, &Result.NormalTextureArray[0].GpuDescriptor);
        }
    }

    {
        // TODO: Я не обчислюю дотичні/бідотичні вектори тутай
        local_global vertex ModelVertices[] =
            {
                // NOTE: Front Face
                { V3(-0.5f, -0.5f, -0.5f), V2(0, 0), V3(0, 0, 1) },
                { V3(-0.5f, 0.5f, -0.5f), V2(1, 0), V3(0, 0, 1) },
                { V3(0.5f, 0.5f, -0.5f), V2(1, 1), V3(0, 0, 1) },
                { V3(0.5f, -0.5f, -0.5f), V2(0, 1), V3(0, 0, 1) },

                // NOTE: Back Face
                { V3(-0.5f, -0.5f, 0.5f), V2(0, 0), V3(0, 0, -1) },
                { V3(-0.5f, 0.5f, 0.5f), V2(1, 0), V3(0, 0, -1) },
                { V3(0.5f, 0.5f, 0.5f), V2(1, 1), V3(0, 0, -1) }, 
                { V3(0.5f, -0.5f, 0.5f), V2(0, 1), V3(0, 0, -1) },

                // NOTE: Left Face
                { V3(-0.5f, 0.5f, -0.5f), V2(0, 0), V3(-1, 0, 0) },
                { V3(-0.5f, -0.5f, -0.5f), V2(1, 0), V3(-1, 0, 0) },
                { V3(-0.5f, -0.5f, 0.5f), V2(1, 1), V3(-1, 0, 0) },
                { V3(-0.5f, 0.5f, 0.5f), V2(0, 1), V3(-1, 0, 0) },

                // NOTE: Right Face
                { V3(0.5f, 0.5f, -0.5f), V2(0, 0), V3(1, 0, 0) },
                { V3(0.5f, 0.5f, 0.5f), V2(1, 0), V3(1, 0, 0) },
                { V3(0.5f, -0.5f, 0.5f), V2(1, 1), V3(1, 0, 0) },
                { V3(0.5f, -0.5f, -0.5f), V2(0, 1), V3(1, 0, 0) },

                // NOTE: Top Face
                { V3(-0.5f, 0.5f, -0.5f), V2(0, 0), V3(0, 1, 0) },
                { V3(-0.5f, 0.5f, 0.5f), V2(1, 0), V3(0, 1, 0) },
                { V3(0.5f, 0.5f, 0.5f), V2(1, 1), V3(0, 1, 0) },
                { V3(0.5f, 0.5f, -0.5f), V2(0, 1), V3(0, 1, 0) },

                // NOTE: Bottom Face
                { V3(-0.5f, -0.5f, -0.5f), V2(0, 0), V3(0, -1, 0) },
                { V3(0.5f, -0.5f, -0.5f), V2(0, 1), V3(0, -1, 0) },
                { V3(0.5f, -0.5f, 0.5f), V2(1, 1), V3(0, -1, 0) },
                { V3(-0.5f, -0.5f, 0.5f), V2(1, 0), V3(0, -1, 0) },
            };

        local_global u32 ModelIndices[] =
            {
                // NOTE: Front Face
                0, 1, 2,
                2, 3, 0,

                // NOTE: Back Face
                4, 7, 6,
                6, 5, 4,

                // NOTE: Left face
                8, 9, 10,
                10, 11, 8,

                // NOTE: Right face
                12, 13, 14,
                14, 15, 12,

                // NOTE: Top face
                16, 17, 18,
                18, 19, 16,

                // NOTE: Bottom face
                20, 21, 22,
                22, 23, 20,
            };

        
        Result.VertexCount = ArrayCount(ModelVertices);
        Result.IndexCount = ArrayCount(ModelIndices);

        Result.NumMeshes = 1;
        Result.MeshArray = (mesh*)malloc(sizeof(mesh));
        {
            mesh Mesh = {};
            Mesh.IndexOffset = 0;
            Mesh.IndexCount = Result.IndexCount;
            Mesh.VertexOffset = 0;
            Mesh.VertexCount = Result.VertexCount;
            Mesh.TextureId = 0;
            Result.MeshArray[0] = Mesh;
        }

        Result.VertexArray = ModelVertices;
        Result.IndexArray = ModelIndices;
        Result.GpuVertexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.VertexCount * sizeof(vertex), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ModelVertices);
        Result.GpuIndexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.IndexCount * sizeof(u32), D3D12_RESOURCE_STATE_INDEX_BUFFER, ModelIndices);
    }
    
    return Result;
}

model AssetLoadModel(dx12_rasterizer* Dx12Rasterizer, char* FolderPath, char* FileName)
{
    model Result = {};

    char* FilePath = CombineStrings(FolderPath, FileName);
    
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(FilePath, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_OptimizeMeshes);
    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
    {
        const char* Error = Importer.GetErrorString();
        InvalidCodePath;
    }

    free(FilePath);
    
    u32* TextureMappingTable = (u32*)malloc(sizeof(u32) * Scene->mNumMaterials);
    for (u32 MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId];

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            TextureMappingTable[MaterialId] = Result.NumTextures;
            Result.NumTextures += 1;
        }
        else
        {
            TextureMappingTable[MaterialId] = 0xFFFFFFFF;
        }
    }

    Result.ColorTextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);
    Result.NormalTextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);

    u32 CurrTextureId = 0;
    for (u32 MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId];

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            // NOTE: Завантажуємо дифузну текстуру
            {
                aiString TextureName = {};
                CurrMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TextureName);

                texture* CurrTexture = Result.ColorTextureArray + CurrTextureId;

                char* TexturePath = CombineStrings(FolderPath, TextureName.C_Str());
            
                i32 NumChannels = 0;
                u32* UnFlippedTexels = (u32*)stbi_load(TexturePath, &CurrTexture->Width, &CurrTexture->Height, &NumChannels, 4);

                CurrTexture->Texels = (u32*)malloc(sizeof(u32) * CurrTexture->Width * CurrTexture->Height);

                for (u32 Y = 0; Y < CurrTexture->Height; ++Y)
                {
                    for (u32 X = 0; X < CurrTexture->Width; ++X)
                    {
                        u32 PixelId = Y * CurrTexture->Width + X;
                        CurrTexture->Texels[PixelId] = UnFlippedTexels[(CurrTexture->Height - Y - 1) * CurrTexture->Width + X];
                    }
                }

                // NOTE: Копюємо дані до Upload Heap
                Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (u8*)CurrTexture->Texels,
                                  &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor);
            
                stbi_image_free(UnFlippedTexels);
            }
            
            // NOTE: Завантажуємо текстуру нормалі
            {
                aiString TextureName = {};
                CurrMaterial->GetTexture(aiTextureType_NORMALS, 0, &TextureName);
                
                texture* CurrTexture = Result.NormalTextureArray + CurrTextureId;

                if (CurrMaterial->GetTextureCount(aiTextureType_NORMALS) == 0)
                {
                    *CurrTexture = AssetCreateDefaultNormalMap();
                    
                    // NOTE: Копюємо дані до Upload Heap
                    Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (u8*)CurrTexture->Texels,
                                      &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor, true);
                }
                else
                {
                    char* TexturePath = CombineStrings(FolderPath, TextureName.C_Str());
                    
                    i32 NumChannels = 0;
                    u32* UnFlippedTexels = (u32*)stbi_load(TexturePath, &CurrTexture->Width, &CurrTexture->Height, &NumChannels, 4);
                    CurrTexture->Texels = (u32*)malloc(sizeof(u32) * CurrTexture->Width * CurrTexture->Height);
                    
                    for (u32 Y = 0; Y < CurrTexture->Height; ++Y)
                    {
                        for (u32 X = 0; X < CurrTexture->Width; ++X)
                        {
                            u32 PixelId = Y * CurrTexture->Width + X;
                            CurrTexture->Texels[PixelId] = UnFlippedTexels[(CurrTexture->Height - Y - 1) * CurrTexture->Width + X];
                        }
                    }
                    
                    // NOTE: Копюємо дані до Upload Heap
                    Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (u8*)CurrTexture->Texels,
                                      &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor, true);
                    
                    stbi_image_free(UnFlippedTexels);
                }
            }
            
            CurrTextureId += 1;
        }
    }
    
    Result.NumMeshes = Scene->mNumMeshes;
    Result.MeshArray = (mesh*)malloc(sizeof(mesh) * Result.NumMeshes);
    for (u32 MeshId = 0; MeshId < Result.NumMeshes; ++MeshId)
    {
        aiMesh* SrcMesh = Scene->mMeshes[MeshId];
        mesh* DstMesh = Result.MeshArray + MeshId;

        DstMesh->TextureId = TextureMappingTable[SrcMesh->mMaterialIndex];
        DstMesh->IndexOffset = Result.IndexCount;
        DstMesh->VertexOffset = Result.VertexCount;
        DstMesh->IndexCount = SrcMesh->mNumFaces * 3;
        DstMesh->VertexCount = SrcMesh->mNumVertices;

        Result.IndexCount += DstMesh->IndexCount;
        Result.VertexCount += DstMesh->VertexCount;
    }

    Result.VertexArray = (vertex*)malloc(sizeof(vertex) * Result.VertexCount);
    Result.IndexArray = (u32*)malloc(sizeof(u32) * Result.IndexCount);

    f32 MinDistAxis = FLT_MAX;
    f32 MaxDistAxis = FLT_MIN;
    for (u32 MeshId = 0; MeshId < Result.NumMeshes; ++MeshId)
    {
        aiMesh* SrcMesh = Scene->mMeshes[MeshId];
        mesh* DstMesh = Result.MeshArray + MeshId;

        for (u32 VertexId = 0; VertexId < DstMesh->VertexCount; ++VertexId)
        {
            vertex* CurrVertex = Result.VertexArray + DstMesh->VertexOffset + VertexId;
            CurrVertex->Pos = V3(SrcMesh->mVertices[VertexId].x,
                                 SrcMesh->mVertices[VertexId].y,
                                 SrcMesh->mVertices[VertexId].z);

            MinDistAxis = min(MinDistAxis, min(CurrVertex->Pos.x, min(CurrVertex->Pos.y, CurrVertex->Pos.z)));
            MaxDistAxis = max(MaxDistAxis, max(CurrVertex->Pos.x, max(CurrVertex->Pos.y, CurrVertex->Pos.z)));

            CurrVertex->Normal = V3(SrcMesh->mNormals[VertexId].x,
                                    SrcMesh->mNormals[VertexId].y,
                                    SrcMesh->mNormals[VertexId].z);

            CurrVertex->Tangent = V3(SrcMesh->mTangents[VertexId].x,
                                     SrcMesh->mTangents[VertexId].y,
                                     SrcMesh->mTangents[VertexId].z);
            
            CurrVertex->BiTangent = V3(SrcMesh->mBitangents[VertexId].x,
                                       SrcMesh->mBitangents[VertexId].y,
                                       SrcMesh->mBitangents[VertexId].z);
            
            if (SrcMesh->mTextureCoords[0])
            {
                CurrVertex->Uv = V2(SrcMesh->mTextureCoords[0][VertexId].x,
                                    SrcMesh->mTextureCoords[0][VertexId].y);
            }
            else
            {
                CurrVertex->Uv = V2(0, 0);
            }
        }

        for (u32 FaceId = 0; FaceId < SrcMesh->mNumFaces; ++FaceId)
        {
            u32* CurrIndices = Result.IndexArray + DstMesh->IndexOffset + FaceId * 3;

            CurrIndices[0] = SrcMesh->mFaces[FaceId].mIndices[0];
            CurrIndices[1] = SrcMesh->mFaces[FaceId].mIndices[1];
            CurrIndices[2] = SrcMesh->mFaces[FaceId].mIndices[2];
        }
    }
    
    for (u32 VertexId = 0; VertexId < Result.VertexCount; ++VertexId)
    {
        vertex* CurrVertex = Result.VertexArray + VertexId;

        // NOTE: Результат є між [0, 1]
        CurrVertex->Pos = (CurrVertex->Pos - V3(MinDistAxis)) / (MaxDistAxis - MinDistAxis);
        CurrVertex->Pos = CurrVertex->Pos - V3(0.5f);
    }

    Result.GpuVertexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.VertexCount * sizeof(vertex), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, Result.VertexArray);
    Result.GpuIndexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.IndexCount * sizeof(u32), D3D12_RESOURCE_STATE_INDEX_BUFFER, Result.IndexArray);
    
    return Result;
}
