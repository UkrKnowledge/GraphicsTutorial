
i64 Sign(i64 X)
{
    i64 Result = (X > 0) - (X < 0);
    return Result;
}

//
// NOTE: I32_X4
//

i32_x4 I32X4(f32 X, f32 Y, f32 Z, f32 W)
{
    i32_x4 Result = {};
    Result.Vals = _mm_set_epi32(W, Z, Y, X);
    return Result;
}

i32_x4 I32X4(i32 X)
{
    i32_x4 Result = {};
    Result.Vals = _mm_set1_epi32(X);
    return Result;
}

i32_x4 I32X4(f32_x4 A)
{
    i32_x4 Result = {};
    Result.Vals = _mm_cvtps_epi32(A.Vals);
    return Result;
}

i32_x4 I32X4ReInterpret(f32_x4 X)
{
    i32_x4 Result = {};
    Result.Vals = _mm_castps_si128(X.Vals);
    return Result;
}

i32_x4 I32X4Load(i32* Ptr)
{
    i32_x4 Result = {};
    Result.Vals = _mm_loadu_si128((__m128i*)Ptr);
    return Result;
}

i32_x4 I32X4Gather(i32* Ptr, i32_x4 Offsets)
{
    i32_x4 Result = I32X4(Ptr[Offsets.e[0]], Ptr[Offsets.e[1]], Ptr[Offsets.e[2]], Ptr[Offsets.e[3]]);
    return Result;
}

void I32X4Store(i32* Ptr, i32_x4 Values)
{
    _mm_storeu_si128((__m128i*)Ptr, Values.Vals);
}

i32_x4 operator+(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_add_epi32(A.Vals, B.Vals);
    return Result;
}

i32_x4 operator+(i32_x4 A, i32 B)
{
    i32_x4 Result = A + I32X4(B);
    return Result;
}

i32_x4 operator+=(i32_x4& A, i32_x4 B)
{
    A = A + B;
    return A;
}

i32_x4 operator-(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_sub_epi32(A.Vals, B.Vals);
    return Result;
}

i32_x4 operator*(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_mullo_epi32(A.Vals, B.Vals);
    return Result;
}

i32_x4 operator*(i32_x4 A, i32 B)
{
    i32_x4 Result = A * I32X4(B);
    return Result;
}

i32_x4 operator|(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_or_si128(A.Vals, B.Vals);
    return Result;
}

i32_x4 operator&(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_and_si128(A.Vals, B.Vals);
    return Result;
}

i32_x4 operator&(i32_x4 A, i32 B)
{
    i32_x4 Result = A & I32X4(B);
    return Result;
}

i32_x4 operator>=(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_or_si128(_mm_cmpgt_epi32(A.Vals, B.Vals), _mm_cmpeq_epi32(A.Vals, B.Vals));
    return Result;
}

i32_x4 operator<(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_cmplt_epi32(A.Vals, B.Vals);
    return Result;
}

i32_x4 operator<(i32_x4 A, i32 B)
{
    i32_x4 Result = A < I32X4(B);
    return Result;
}

i32_x4 operator<<(i32_x4 A, i32 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_sll_epi32(A.Vals, _mm_set_epi32(0, 0, 0, B));
    return Result;
}

i32_x4 operator>>(i32_x4 A, i32 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_srl_epi32(A.Vals, _mm_set_epi32(0, 0, 0, B));
    return Result;
}

i32_x4 operator>=(i32_x4 A, i32 B)
{
    i32_x4 Result = A >= I32X4(B);
    return Result;
}

i32_x4 AndNot(i32_x4 Mask, i32_x4 Value)
{
    i32_x4 Result = {};
    Result.Vals = _mm_andnot_si128(Mask.Vals, Value.Vals);
    return Result;
}

i32_x4 Min(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_min_epi32(A.Vals, B.Vals);
    return Result;
}

i32_x4 Min(i32_x4 A, i32 B)
{
    i32_x4 Result = Min(A, I32X4(B));
    return Result;
}

i32_x4 Max(i32_x4 A, i32_x4 B)
{
    i32_x4 Result = {};
    Result.Vals = _mm_max_epi32(A.Vals, B.Vals);
    return Result;
}

i32_x4 Max(i32_x4 A, i32 B)
{
    i32_x4 Result = Max(A, I32X4(B));
    return Result;
}

//
// NOTE: F32_X4
//

f32_x4 F32X4(f32 X)
{
    f32_x4 Result = {};
    Result.Vals = _mm_set1_ps(X);
    return Result;
}

f32_x4 F32X4(i32_x4 X)
{
    f32_x4 Result = {};
    Result.Vals = _mm_cvtepi32_ps(X.Vals);
    return Result;
}

f32_x4 F32X4ReInterpret(i32_x4 X)
{
    f32_x4 Result = {};
    Result.Vals = _mm_castsi128_ps(X.Vals);
    return Result;
}

f32_x4 F32X4Load(f32* Ptr)
{
    f32_x4 Result = {};
    Result.Vals = _mm_loadu_ps(Ptr);
    return Result;
}

void F32X4Store(f32* Ptr, f32_x4 Values)
{
    _mm_storeu_ps(Ptr, Values.Vals);
}

f32_x4 operator+(f32_x4 A, f32_x4 B)
{
    f32_x4 Result = {};
    Result.Vals = _mm_add_ps(A.Vals, B.Vals);
    return Result;
}

f32_x4 operator+(f32 A, f32_x4 B)
{
    f32_x4 Result = F32X4(A) + B;
    return Result;
}

f32_x4 operator-(f32_x4 A)
{
    f32_x4 Result = {};
    Result.Vals = _mm_mul_ps(A.Vals, _mm_set1_ps(-1.0f));
    return Result;
}

f32_x4 operator-(f32_x4 A, f32_x4 B)
{
    f32_x4 Result = {};
    Result.Vals = _mm_sub_ps(A.Vals, B.Vals);
    return Result;
}

f32_x4 operator-(f32_x4 A, f32 B)
{
    f32_x4 Result = A - F32X4(B);
    return Result;
}

f32_x4 operator-(f32 A, f32_x4 B)
{
    f32_x4 Result = F32X4(A) - B;
    return Result;
}

f32_x4 operator*(f32_x4 A, f32_x4 B)
{
    f32_x4 Result = {};
    Result.Vals = _mm_mul_ps(A.Vals, B.Vals);
    return Result;
}

f32_x4 operator*(f32_x4 A, f32 B)
{
    f32_x4 Result = A * F32X4(B);
    return Result;
}

f32_x4 operator/(f32_x4 A, f32_x4 B)
{
    f32_x4 Result = {};
    Result.Vals = _mm_div_ps(A.Vals, B.Vals);
    return Result;
}

f32_x4 operator/(f32_x4 A, f32 B)
{
    f32_x4 Result = A / F32X4(B);
    return Result;
}

f32_x4 operator&(f32_x4 A, f32_x4 B)
{
    f32_x4 Result = {};
    Result.Vals = _mm_and_ps(A.Vals, B.Vals);
    return Result;
}

f32_x4 operator<(f32_x4 A, f32_x4 B)
{
    f32_x4 Result = {};
    Result.Vals = _mm_cmplt_ps(A.Vals, B.Vals);
    return Result;
}

f32_x4 AndNot(f32_x4 Mask, f32_x4 Value)
{
    f32_x4 Result = {};
    Result.Vals = _mm_andnot_ps(Mask.Vals, Value.Vals);
    return Result;
}

f32_x4 Floor(f32_x4 A)
{
    f32_x4 Result = {};
    Result.Vals = _mm_floor_ps(A.Vals);
    return Result;
}

//
// NOTE: V2
//

v2 V2(f32 Arg)
{
    v2 Result = {};
    Result.x = Arg;
    Result.y = Arg;
    return Result;
}

v2 V2(f32 X, f32 Y)
{
    v2 Result = {};
    Result.x = X;
    Result.y = Y;
    return Result;
}

v2 operator+(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return Result;
}

v2 operator-(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

v2 operator*(f32 A, v2 B)
{
    v2 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    return Result;
}

v2 operator*(v2 B, f32 A)
{
    v2 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    return Result;
}

v2 operator*(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    return Result;
}

v2 operator*=(v2& A, f32 B)
{
    A = A * B;
    return A;
}

v2 operator/(v2 A, f32 B)
{
    v2 Result = {};
    Result.x = A.x / B;
    Result.y = A.y / B;
    return Result;
}

v2 operator/=(v2& A, f32 B)
{
    A = A / B;
    return A;
}

//
// NOTE: V2_X4
//

v2_x4 V2X4(v2i_x4 A)
{
    v2_x4 Result = {};
    Result.x = F32X4(A.x);
    Result.y = F32X4(A.y);
    return Result;
}

v2_x4 operator+(v2_x4 A, v2_x4 B)
{
    v2_x4 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return Result;
}

v2_x4 operator-(v2_x4 A, v2_x4 B)
{
    v2_x4 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

v2_x4 operator-(v2_x4 A, v2 B)
{
    v2_x4 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

v2_x4 operator*(f32_x4 A, v2 B)
{
    v2_x4 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    return Result;
}

v2_x4 operator*(v2_x4 A, v2 B)
{
    v2_x4 Result = {};
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    return Result;
}

v2_x4 operator/(v2_x4 A, f32_x4 B)
{
    v2_x4 Result = {};
    Result.x = A.x / B;
    Result.y = A.y / B;
    return Result;
}

v2_x4 operator/(v2_x4 A, v2 B)
{
    v2_x4 Result = {};
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    return Result;
}

v2_x4 Floor(v2_x4 A)
{
    v2_x4 Result = {};
    Result.x = Floor(A.x);
    Result.y = Floor(A.y);
    return Result;
}

//
// NOTE: V2I
//

v2i V2I(f32 X, f32 Y)
{
    v2i Result = {};
    Result.x = (i32)X;
    Result.y = (i32)Y;
    return Result;
}

v2i V2I(v2 A)
{
    v2i Result = {};
    Result.x = (i32)A.x;
    Result.y = (i32)A.y;
    return Result;
}

v2i V2I_F24_8(v2 A)
{
    v2i Result = {};
    Result.x = (i32)round(A.x * 256.0f);
    Result.y = (i32)round(A.y * 256.0f);
    return Result;
}

v2i operator+(v2i A, v2i B)
{
    v2i Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return Result;
}

v2i operator-(v2i A, v2i B)
{
    v2i Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return Result;
}

//
// NOTE: V2IX4
//

v2i_x4 V2IX4(f32_x4 A, f32_x4 B)
{
    v2i_x4 Result = {};
    Result.x = I32X4(A);
    Result.y = I32X4(B);
    return Result;
}

v2i_x4 V2IX4(v2_x4 A)
{
    v2i_x4 Result = {};
    Result.x = I32X4(A.x);
    Result.y = I32X4(A.y);
    return Result;
}

v2i_x4 operator+(v2i_x4 A, v2i B)
{
    v2i_x4 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return Result;
}

//
// NOTE: V3
//

v3 V3(f32 X, f32 Y, f32 Z)
{
    v3 Result = {};
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    return Result;
}

v3 V3(f32 A)
{
    v3 Result = {};
    Result.x = A;
    Result.y = A;
    Result.z = A;
    return Result;
}

v3 operator+(v3 A, v3 B)
{
    v3 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    return Result;
}

v3 operator+=(v3& A, v3 B)
{
    A = A + B;
    return A;
}

v3 operator-(v3 A)
{
    v3 Result = {};
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    return Result;
}

v3 operator-(v3 A, v3 B)
{
    v3 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    return Result;
}

v3 operator-=(v3& A, v3 B)
{
    A = A - B;
    return A;
}

v3 operator*(f32 A, v3 B)
{
    v3 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    return Result;
}

v3 operator*(v3 B, f32 A)
{
    v3 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    return Result;
}

v3 operator*=(v3& A, f32 B)
{
    A = A * B;
    return A;
}

v3 operator/(v3 B, f32 A)
{
    v3 Result = {};
    Result.x = B.x / A;
    Result.y = B.y / A;
    Result.z = B.z / A;
    return Result;
}

v3 operator/=(v3& A, f32 B)
{
    A = A / B;
    return A;
}

v3 Normalize(v3 A)
{
    f32 Length = sqrt(A.x * A.x + A.y * A.y + A.z * A.z);
    v3 Result = A / Length;
    return Result;
}

v3 Lerp(v3 A, v3 B, f32 T)
{
    v3 Result = (1.0f - T) * A + T * B;
    return Result;
}

//
// NOTE: V3
//

v3_x4 V3X4(f32 A)
{
    v3_x4 Result = {};
    Result.x = F32X4(A);
    Result.y = F32X4(A);
    Result.z = F32X4(A);
    return Result;
}

v3_x4 operator+(v3_x4 A, v3_x4 B)
{
    v3_x4 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    return Result;
}

v3_x4 operator*(v3_x4 A, f32 B)
{
    v3_x4 Result = {};
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    return Result;
}

v3_x4 operator*(f32_x4 A, v3_x4 B)
{
    v3_x4 Result = {};
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    return Result;
}

v3_x4 operator/(v3_x4 A, f32 B)
{
    v3_x4 Result = {};
    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;
    return Result;
}

v3_x4 Lerp(v3_x4 A, v3_x4 B, f32_x4 T)
{
    v3_x4 Result = (1.0f - T) * A + T * B;
    return Result;
}

//
// NOTE: V4
//

v4 V4(f32 X, f32 Y, f32 Z, f32 W)
{
    v4 Result = {};
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return Result;
}

v4 V4(v3 A, f32 W)
{
    v4 Result = {};
    Result.xyz = A;
    Result.w = W;
    return Result;
}

v4 operator+(v4 A, v4 B)
{
    v4 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    return Result;
}

v4 operator*(v4 A, f32 B)
{
    v4 Result = {};
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;
    return Result;
}

v4 operator*(f32 B, v4 A)
{
    v4 Result = {};
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;
    return Result;
}

//
// NOTE: M4
//

m4 IdentityM4()
{
    m4 Result = {};
    Result.v[0].x = 1.0f;
    Result.v[1].y = 1.0f;
    Result.v[2].z = 1.0f;
    Result.v[3].w = 1.0f;
    return Result;
}

v4 operator*(m4 A, v4 B)
{
    v4 Result = A.v[0] * B.x + A.v[1] * B.y + A.v[2] * B.z + A.v[3] * B.w;
    return Result;
}

m4 operator*(m4 A, m4 B)
{
    m4 Result = {};
    Result.v[0] = A * B.v[0];
    Result.v[1] = A * B.v[1];
    Result.v[2] = A * B.v[2];
    Result.v[3] = A * B.v[3];
    return Result;
}

m4 ScaleMatrix(f32 X, f32 Y, f32 Z)
{
    m4 Result = IdentityM4();
    Result.v[0].x = X;
    Result.v[1].y = Y;
    Result.v[2].z = Z;
    return Result;
}

m4 RotationMatrix(f32 X, f32 Y, f32 Z)
{
    m4 Result = {};

    m4 RotateX = IdentityM4();
    RotateX.v[1].y = cos(X);
    RotateX.v[2].y = -sin(X);
    RotateX.v[1].z = sin(X);
    RotateX.v[2].z = cos(X);

    m4 RotateY = IdentityM4();
    RotateY.v[0].x = cos(Y);
    RotateY.v[2].x = -sin(Y);
    RotateY.v[0].z = sin(Y);
    RotateY.v[2].z = cos(Y);

    m4 RotateZ = IdentityM4();
    RotateZ.v[0].x = cos(Z);
    RotateZ.v[1].x = -sin(Z);
    RotateZ.v[0].y = sin(Z);
    RotateZ.v[1].y = cos(Z);

    Result = RotateZ * RotateY * RotateX;
    return Result;
}

m4 TranslationMatrix(f32 X, f32 Y, f32 Z)
{
    m4 Result = IdentityM4();
    Result.v[3].xyz = V3(X, Y, Z);
    return Result;
}

m4 TranslationMatrix(v3 Pos)
{
    m4 Result = TranslationMatrix(Pos.x, Pos.y, Pos.z);
    return Result;
}

m4 PerspectiveMatrix(f32 Fov, f32 AspectRatio, f32 NearZ, f32 FarZ)
{
    // NOTE: Ми очікуємо що Fov є у градусів
    m4 Result = {};

    f32 FovRadians = (Fov / 360.0f) * 2.0f * Pi32;
    
    Result.v[0].x = 1.0f / (AspectRatio * tan(FovRadians * 0.5f));
    Result.v[1].y = 1.0f / (tan(FovRadians * 0.5f));
    Result.v[2].z = -FarZ / (NearZ - FarZ);
    Result.v[3].z = NearZ * FarZ / (NearZ - FarZ);
    Result.v[2].w = 1.0f;

    return Result;
}
