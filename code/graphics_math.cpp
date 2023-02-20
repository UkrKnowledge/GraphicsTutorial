
//
// NOTE: V2
//

v2 V2(f32 X, f32 Y)
{
    v2 Result;

    Result.x = X;
    Result.y = Y;
    
    return Result;
}

v2 V2(u32 X, u32 Y)
{
    v2 Result;

    Result.x = (f32)X;
    Result.y = (f32)Y;
    
    return Result;
}

v2 operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return Result;
}

v2 operator*(f32 B, v2 A)
{
    v2 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;

    return Result;
}

v2 operator*(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;

    return Result;
}

v2 operator/(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x / B;
    Result.y = A.y / B;

    return Result;
}

//
// NOTE: V3 
//

inline v3 V3(f32 X, f32 Y, f32 Z)
{
    v3 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    
    return Result;
}

v3 operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}
