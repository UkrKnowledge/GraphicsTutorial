
sw_rasterizer SwRasterizerCreate(HWND WindowHandle, u32 Width, u32 Height)
{
    sw_rasterizer Result = {};
    Result.DeviceContext = GetDC(WindowHandle);
    Result.FrameBufferWidth = Width;
    Result.FrameBufferStride = Result.FrameBufferWidth + 3;
    Result.FrameBufferHeight = Height;
    Result.FrameBufferPixels = (u32*)malloc(sizeof(u32) * Result.FrameBufferStride *
                                            Result.FrameBufferHeight);
    Result.DepthBuffer = (f32*)malloc(sizeof(f32) * Result.FrameBufferStride *
                                      Result.FrameBufferHeight);

    return Result;
}

v2 NdcToPixels(sw_rasterizer* Rasterizer, v2 NdcPos)
{
    v2 Result = {};
    Result = 0.5f * (NdcPos + V2(1.0f, 1.0f));
    Result = V2(Rasterizer->FrameBufferWidth, Rasterizer->FrameBufferHeight) * Result;
    
    return Result;
}

i64 CrossProduct2d(v2i A, v2i B)
{
    i64 Result = i64(A.x) * i64(B.y) - i64(A.y) * i64(B.x);
    return Result;
}

v3 ColorU32ToRgb(u32 Color)
{
    v3 Result = {};
    Result.r = (Color >> 16) & 0xFF;
    Result.g = (Color >> 8) & 0xFF;
    Result.b = (Color >> 0) & 0xFF;
    Result /= 255.0f;
    return Result;
}

u32 ColorRgbToU32(v3 Color)
{
    Color *= 255.0f;
    u32 Result = ((u32)0xFF << 24) | ((u32)Color.r << 16) | ((u32)Color.g << 8) | (u32)Color.b;
    return Result;
}

v3_x4 ColorI32ToRgb(i32_x4 Color)
{
    v3_x4 Result = {};
    Result.b = F32X4((Color >> 16) & 0xFF);
    Result.g = F32X4((Color >> 8) & 0xFF);
    Result.r = F32X4((Color >> 0) & 0xFF);
    Result = Result / 255.0f;
    return Result;
}

i32_x4 ColorRgbToI32(v3_x4 Color)
{
    Color = Color * 255.0f;
    i32_x4 Result = (I32X4(0xFF) << 24) | (I32X4(Color.r) << 16) | (I32X4(Color.g) << 8) | I32X4(Color.b);
    return Result;
}

void DrawTriangle(sw_rasterizer* Rasterizer, clip_vertex Vertex0, clip_vertex Vertex1, clip_vertex Vertex2, 
                  texture Texture, sampler Sampler)
{
    
    Vertex0.Pos.w = 1.0f / Vertex0.Pos.w;
    Vertex1.Pos.w = 1.0f / Vertex1.Pos.w;
    Vertex2.Pos.w = 1.0f / Vertex2.Pos.w;
    
    Vertex0.Pos.xyz *= Vertex0.Pos.w;
    Vertex1.Pos.xyz *= Vertex1.Pos.w;
    Vertex2.Pos.xyz *= Vertex2.Pos.w;

    Vertex0.Uv *= Vertex0.Pos.w;
    Vertex1.Uv *= Vertex1.Pos.w;
    Vertex2.Uv *= Vertex2.Pos.w;
    
    v2 PointAF = NdcToPixels(Rasterizer, Vertex0.Pos.xy);
    v2 PointBF = NdcToPixels(Rasterizer, Vertex1.Pos.xy);
    v2 PointCF = NdcToPixels(Rasterizer, Vertex2.Pos.xy);

    i32 MinX = min(min((i32)PointAF.x, (i32)PointBF.x), (i32)PointCF.x);
    i32 MaxX = max(max((i32)round(PointAF.x), (i32)round(PointBF.x)), (i32)round(PointCF.x));
    i32 MinY = min(min((i32)PointAF.y, (i32)PointBF.y), (i32)PointCF.y);
    i32 MaxY = max(max((i32)round(PointAF.y), (i32)round(PointBF.y)), (i32)round(PointCF.y));

#if 0
    MinX = max(0, MinX);
    MinX = min(GlobalState.FrameBufferWidth - 1, MinX);
    MaxX = max(0, MaxX);
    MaxX = min(GlobalState.FrameBufferWidth - 1, MaxX);
    MinY = max(0, MinY);
    MinY = min(GlobalState.FrameBufferHeight - 1, MinY);
    MaxY = max(0, MaxY);
    MaxY = min(GlobalState.FrameBufferHeight - 1, MaxY);
#endif

    v2i PointA = V2I_F24_8(PointAF);
    v2i PointB = V2I_F24_8(PointBF);
    v2i PointC = V2I_F24_8(PointCF);
    
    v2i Edge0 = PointB - PointA;
    v2i Edge1 = PointC - PointB;
    v2i Edge2 = PointA - PointC;

    b32 IsTopLeft0 = (Edge0.y > 0) || (Edge0.x > 0 && Edge0.y == 0);
    b32 IsTopLeft1 = (Edge1.y > 0) || (Edge1.x > 0 && Edge1.y == 0);
    b32 IsTopLeft2 = (Edge2.y > 0) || (Edge2.x > 0 && Edge2.y == 0);
    
    f32_x4 BaryCentricDiv = F32X4(256.0f / f32(CrossProduct2d(PointB - PointA, PointC - PointA)));

    i32_x4 Edge0DiffX = I32X4(Edge0.y);
    i32_x4 Edge1DiffX = I32X4(Edge1.y);
    i32_x4 Edge2DiffX = I32X4(Edge2.y);

    i32_x4 Edge0DiffY = I32X4(-Edge0.x);
    i32_x4 Edge1DiffY = I32X4(-Edge1.x);
    i32_x4 Edge2DiffY = I32X4(-Edge2.x);

    i32_x4 Edge0RowY = {};
    i32_x4 Edge1RowY = {};
    i32_x4 Edge2RowY = {};
    {
        v2i StartPos = V2I_F24_8(V2(MinX, MinY) + V2(0.5f, 0.5f));
        i64 Edge0RowY64 = CrossProduct2d(StartPos - PointA, Edge0);
        i64 Edge1RowY64 = CrossProduct2d(StartPos - PointB, Edge1);
        i64 Edge2RowY64 = CrossProduct2d(StartPos - PointC, Edge2);

        i32 Edge0RowY32 = i32((Edge0RowY64 + Sign(Edge0RowY64) * 128) / 256) - (IsTopLeft0 ? 0 : -1);
        i32 Edge1RowY32 = i32((Edge1RowY64 + Sign(Edge1RowY64) * 128) / 256) - (IsTopLeft1 ? 0 : -1);
        i32 Edge2RowY32 = i32((Edge2RowY64 + Sign(Edge2RowY64) * 128) / 256) - (IsTopLeft2 ? 0 : -1);

        Edge0RowY = I32X4(Edge0RowY32) + I32X4(0, 1, 2, 3) * Edge0DiffX;
        Edge1RowY = I32X4(Edge1RowY32) + I32X4(0, 1, 2, 3) * Edge1DiffX;
        Edge2RowY = I32X4(Edge2RowY32) + I32X4(0, 1, 2, 3) * Edge2DiffX;
    }

    Edge0DiffX = Edge0DiffX * I32X4(4);
    Edge1DiffX = Edge1DiffX * I32X4(4);
    Edge2DiffX = Edge2DiffX * I32X4(4);
    
    for (i32 Y = MinY; Y <= MaxY; ++Y)
    {
        i32_x4 Edge0RowX = Edge0RowY;
        i32_x4 Edge1RowX = Edge1RowY;
        i32_x4 Edge2RowX = Edge2RowY;
        
        for (i32 X = MinX; X <= MaxX; X += 4)
        {
            u32 PixelId = Y * Rasterizer->FrameBufferStride + X;
            i32* ColorPtr = (i32*)Rasterizer->FrameBufferPixels + PixelId;
            f32* DepthPtr = Rasterizer->DepthBuffer + PixelId;
            i32_x4 PixelColors = I32X4Load(ColorPtr);
            f32_x4 PixelDepths = F32X4Load(DepthPtr);
            
            i32_x4 EdgeMask = (Edge0RowX | Edge1RowX | Edge2RowX) >= 0;

            if (_mm_movemask_epi8(EdgeMask.Vals) != 0)
            {
                f32_x4 T0 = -F32X4(Edge1RowX) * BaryCentricDiv;
                f32_x4 T1 = -F32X4(Edge2RowX) * BaryCentricDiv;
                f32_x4 T2 = -F32X4(Edge0RowX) * BaryCentricDiv;
                
                f32_x4 DepthZ = Vertex0.Pos.z + T1 * (Vertex1.Pos.z - Vertex0.Pos.z) + T2 * (Vertex2.Pos.z - Vertex0.Pos.z);
                i32_x4 DepthMask = I32X4ReInterpret(DepthZ < PixelDepths);
            
                f32_x4 OneOverW = T0 * Vertex0.Pos.w + T1 * Vertex1.Pos.w + T2 * Vertex2.Pos.w;

                v2_x4 Uv = T0 * Vertex0.Uv + T1 * Vertex1.Uv + T2 * Vertex2.Uv;
                Uv = Uv / OneOverW;
                
                i32_x4 TexelColor = I32X4(0);
                switch (Sampler.Type)
                {
                    case SamplerType_Nearest:
                    {
                        i32_x4 TexelX = I32X4(Floor(Uv.x * (Texture.Width - 1)));
                        i32_x4 TexelY = I32X4(Floor(Uv.y * (Texture.Height - 1)));

                        i32_x4 TexelMask = (TexelX >= 0 & TexelX < Texture.Width &
                                            TexelY >= 0 & TexelY < Texture.Height);

                        TexelX = Max(Min(TexelX, Texture.Width - 1), 0);
                        TexelY = Max(Min(TexelY, Texture.Height - 1), 0);
                        i32_x4 TexelOffsets = TexelY * Texture.Width + TexelX;
                    
                        i32_x4 TrueCase = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
                        i32_x4 FalseCase = I32X4(0xFF00FF00);

                        TexelColor = (TrueCase & TexelMask) + AndNot(TexelMask, FalseCase);
                    } break;

                    case SamplerType_Bilinear:
                    {
#if 0
                        v2_x4 TexelV2 = Uv * V2(Texture.Width, Texture.Height) - V2(0.5f, 0.5f);
                        v2i_x4 TexelPos[4] = {};
                        TexelPos[0] = V2IX4(Floor(TexelV2.x), Floor(TexelV2.y));
                        TexelPos[1] = TexelPos[0] + V2I(1, 0);
                        TexelPos[2] = TexelPos[0] + V2I(0, 1);
                        TexelPos[3] = TexelPos[0] + V2I(1, 1);

                        v3_x4 TexelColors[4] = {};
                        for (u32 TexelId = 0; TexelId < ArrayCount(TexelPos); ++TexelId)
                        {
                            v2i_x4 CurrTexelPos = TexelPos[TexelId];
                            i32_x4 TexelMask = (CurrTexelPos.x >= 0 & CurrTexelPos.x < Texture.Width &
                                                CurrTexelPos.y >= 0 & CurrTexelPos.y < Texture.Height);

                            CurrTexelPos.x = Max(Min(CurrTexelPos.x, Texture.Width - 1), 0);
                            CurrTexelPos.y = Max(Min(CurrTexelPos.y, Texture.Height - 1), 0);
                            i32_x4 TexelOffsets = CurrTexelPos.y * Texture.Width + CurrTexelPos.x;

                            i32_x4 TrueCase = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
                            i32_x4 FalseCase = I32X4(Sampler.BorderColor);
                            i32_x4 TexelColorI32 = (TrueCase & TexelMask) + AndNot(TexelMask, FalseCase);

                            TexelColors[TexelId] = ColorI32ToRgb(TexelColorI32);
                        }
#else
                        v2_x4 TexelV2 = Uv * V2(Texture.Width, Texture.Height) - V2(0.5f, 0.5f);
                        v2i_x4 TexelPos[4] = {};
                        TexelPos[0] = V2IX4(Floor(TexelV2.x), Floor(TexelV2.y));
                        TexelPos[1] = TexelPos[0] + V2I(1, 0);
                        TexelPos[2] = TexelPos[0] + V2I(0, 1);
                        TexelPos[3] = TexelPos[0] + V2I(1, 1);

                        v3_x4 TexelColors[4] = {};
                        for (u32 TexelId = 0; TexelId < ArrayCount(TexelPos); ++TexelId)
                        {
                            v2i_x4 CurrTexelPos = TexelPos[TexelId];
                            {
                                v2_x4 CurrTexelPosF = V2X4(CurrTexelPos);
                                v2_x4 Factor = Floor(CurrTexelPosF / V2(Texture.Width, Texture.Height));
                                CurrTexelPosF = CurrTexelPosF - Factor * V2(Texture.Width, Texture.Height);
                                CurrTexelPos = V2IX4(CurrTexelPosF);
                            }

                            i32_x4 TexelOffsets = CurrTexelPos.y * Texture.Width + CurrTexelPos.x;
                            i32_x4 LoadMask = EdgeMask & DepthMask;
                            TexelOffsets = (TexelOffsets & LoadMask) + AndNot(LoadMask, I32X4(0));
                            i32_x4 TexelColorI32 = I32X4Gather((i32*)Texture.Texels, TexelOffsets);

                            TexelColors[TexelId] = ColorI32ToRgb(TexelColorI32);
                        }
#endif
                        f32_x4 S = TexelV2.x - Floor(TexelV2.x);
                        f32_x4 K = TexelV2.y - Floor(TexelV2.y);

                        v3_x4 Interpolated0 = Lerp(TexelColors[0], TexelColors[1], S);
                        v3_x4 Interpolated1 = Lerp(TexelColors[2], TexelColors[3], S);
                        v3_x4 FinalColor = Lerp(Interpolated0, Interpolated1, K);

                        TexelColor = ColorRgbToI32(FinalColor);
                    } break;
                        
                    default:
                    {
                        InvalidCodePath;
                    }
                }

                i32_x4 FinalMaskI32 = EdgeMask & DepthMask;
                f32_x4 FinalMaskF32 = F32X4ReInterpret(FinalMaskI32);
                i32_x4 OutputColors = (TexelColor & FinalMaskI32) + AndNot(FinalMaskI32, PixelColors);
                f32_x4 OutputDepths = (DepthZ & FinalMaskF32) + AndNot(FinalMaskF32, PixelDepths);

                I32X4Store(ColorPtr, OutputColors);
                F32X4Store(DepthPtr, OutputDepths);
            }
            
            Edge0RowX += Edge0DiffX;
            Edge1RowX += Edge1DiffX;
            Edge2RowX += Edge2DiffX;
        }

        Edge0RowY += Edge0DiffY;
        Edge1RowY += Edge1DiffY;
        Edge2RowY += Edge2DiffY;
    }
}

void DrawTriangle(sw_rasterizer* Rasterizer, v4 ModelVertex0, v4 ModelVertex1, v4 ModelVertex2,
                  v2 ModelUv0, v2 ModelUv1, v2 ModelUv2,
                  texture Texture, sampler Sampler)
{
    clip_result Ping = {};
    Ping.NumTriangles = 1;
    Ping.Vertices[0] = { ModelVertex0, ModelUv0 };
    Ping.Vertices[1] = { ModelVertex1, ModelUv1 };
    Ping.Vertices[2] = { ModelVertex2, ModelUv2 };
    
    clip_result Pong = {};
    
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Left);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Right);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Top);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Bottom);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Near);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Far);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_W);

    for (u32 TriangleId = 0; TriangleId < Pong.NumTriangles; ++TriangleId)
    {
        DrawTriangle(Rasterizer, Pong.Vertices[3*TriangleId + 0], Pong.Vertices[3*TriangleId + 1],
                     Pong.Vertices[3*TriangleId + 2], Texture, Sampler);
    }
}

void DrawModel(sw_rasterizer* Rasterizer, model* Model, m4 Transform, sampler Sampler)
{
    v4* TransformedVertices = (v4*)malloc(sizeof(v4) * Model->VertexCount);
    for (u32 VertexId = 0; VertexId < Model->VertexCount; ++VertexId)
    {
        TransformedVertices[VertexId] = (Transform * V4(Model->VertexArray[VertexId].Pos, 1.0f));
    }

    for (u32 MeshId = 0; MeshId < Model->NumMeshes; ++MeshId)
    {
        mesh* CurrMesh = Model->MeshArray + MeshId;
        texture CurrTexture = Model->TextureArray[CurrMesh->TextureId];
        
        for (u32 IndexId = 0; IndexId < CurrMesh->IndexCount; IndexId += 3)
        {
            u32 Index0 = Model->IndexArray[CurrMesh->IndexOffset + IndexId + 0];
            u32 Index1 = Model->IndexArray[CurrMesh->IndexOffset + IndexId + 1];
            u32 Index2 = Model->IndexArray[CurrMesh->IndexOffset + IndexId + 2];

            v4 Pos0 = TransformedVertices[CurrMesh->VertexOffset + Index0];
            v4 Pos1 = TransformedVertices[CurrMesh->VertexOffset + Index1];
            v4 Pos2 = TransformedVertices[CurrMesh->VertexOffset + Index2];

            v2 Uv0 = Model->VertexArray[CurrMesh->VertexOffset + Index0].Uv;
            v2 Uv1 = Model->VertexArray[CurrMesh->VertexOffset + Index1].Uv;
            v2 Uv2 = Model->VertexArray[CurrMesh->VertexOffset + Index2].Uv;
            
            DrawTriangle(Rasterizer, Pos0, Pos1, Pos2, Uv0, Uv1, Uv2, CurrTexture, Sampler);
        }
    }

    free(TransformedVertices);
}
