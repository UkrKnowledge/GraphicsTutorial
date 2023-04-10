#if !defined(CLIPPER_H)

#define W_CLIPPING_PLANE 0.0001f
#define CLIP_MAX_NUM_VERTICES 384

enum clip_axis
{
    ClipAxis_None,

    ClipAxis_Left,
    ClipAxis_Right,
    ClipAxis_Top,
    ClipAxis_Bottom,
    ClipAxis_Near,
    ClipAxis_Far,
    ClipAxis_W,
};

struct clip_vertex
{
    v4 Pos;
    v2 Uv;
};

struct clip_result
{
    u32 NumTriangles;
    clip_vertex Vertices[CLIP_MAX_NUM_VERTICES];
};

#define CLIPPER_H
#endif
