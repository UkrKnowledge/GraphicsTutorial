#if !defined(GRAPHICS_MATH_H)

union v2
{
    struct
    {
        f32 x, y;
    };

    f32 e[2];
};

union v3
{
    struct
    {
        f32 x, y, z;        
    };
        
    struct
    {
        v2 xy;
        f32 Ignored0_;
    };

    f32 e[3];
};    

#define GRAPHICS_MATH_H
#endif
