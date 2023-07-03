
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

model AssetLoadModel(char* FolderPath, char* FileName)
{
    model Result = {};

    char* FilePath = CombineStrings(FolderPath, FileName);
    
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(FilePath, aiProcess_Triangulate | aiProcess_OptimizeMeshes);
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

    Result.TextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);

    u32 CurrTextureId = 0;
    for (u32 MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId];

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString TextureName = {};
            CurrMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TextureName);

            texture* CurrTexture = Result.TextureArray + CurrTextureId;

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

            stbi_image_free(UnFlippedTexels);
            
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
    
    return Result;
}
