
//~NOTE(tbt): single floats

Function Bool
IsNaN1F(float a)
{
    union AsUint32
    {
        float f;
        uint32_t i;
    } i_from_f =
    {
        .f = a,
    };
    return (i_from_f.i & 0x7fffffff) > 0x7f800000;
}

Function float
Smoothstep1F(float t)
{
    return t*t*(3 - 2*t);
}

Function float
InterpolateLinear1F(float a, float b,
                    float t) // NOTE(tbt): (0.0 <= t <= 1.0)
{
    return a + t*(b - a);
}

Function float
InterpolateExponential1F(float a, float b,
                         float t) // NOTE(tbt): (0.0 <= t <= 1.0)
{
    return InterpolateLinear1F(a, b, t*t);
}

Function float
InterpolateSmooth1F(float a, float b,
                    float t) // NOTE(tbt): (0.0 <= t <= 1.0)
{
    return InterpolateLinear1F(a, b, Smoothstep1F(t));
}

Function float
Min1F(float a, float b)
{
    if(a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

Function float
Max1F(float a, float b)
{
    if(a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

Function float
Clamp1F(float a,
        float min,
        float max)
{
    return (Max1F(min, Min1F(max, a)));
}

Function float
Abs1F(float a)
{
    union { float f; unsigned int u; } u_from_f;
    u_from_f.f = a;
    u_from_f.u &= ~(1 << 31);
    return u_from_f.f;
}

Function float
Pow1F(float a,
      float b)
{
    Assert(0 == Fract1F(b));
    float result;
    int _b = b;
    
    if(_b == 0)
    {
        result = 1;
    }
    else
    {
        float temp = Pow1F(a, _b / 2);
        if(0 == (_b % 2))
        {
            result = temp*temp;
        }
        else if(_b > 0)
        {
            result = a*temp*temp;
        }
        else
        {
            result = (temp*temp) / a;
        }
    }
    
    return result;
}

Function float
Round1F(float a)
{
    return Floor1F(a + 0.5f);
}

Function float
Floor1F(float a)
{
    int i = a;
    return i;
}

Function float
Ceil1F(float a)
{
    int i = a;
    return i + 1;
}

Function float
Fract1F(float a)
{
    int i = a;
    return a - (float)i;
}

Function float
Sqrt1F(float a)
{
    float result;
#if Build_UseSSE2
    __m128 _a;
    _a = _mm_load_ss(&a);
    _a = _mm_sqrt_ss(_a);
    _mm_store_ss(&result, _a);
#else
    int accuracy = 10; // NOTE(tbt): tune lower for speed, higher for accuracy
    
    float guess = 1.0f;
    for(int i = 0; i < accuracy; i += 1)
    {
        guess -= (guess*guess - a) / (2*guess);
    }
    result = guess
#endif
    return result;
}

Function float
ReciprocalSqrt1F(float a)
{
    float result;
#if Build_UseSSE2
    __m128 _a;
    _a = _mm_load_ss(&a);
    _a = _mm_rsqrt_ss(_a);
    _mm_store_ss(&result, _a);
#else
    union { float f; long i; } i_from_f;
    i_from_f.f = a;
    i_from_f.i = 0x5f375a86 - (i_from_f.i >> 1);
    i_from_f.f *= 1.5f - (i_from_f.f*0.5f*i_from_f.f*i_from_f.f);
    result = i_from_f.f;
#endif
    return result;
}

// NOTE(tbt): not very fast or accurate
Function float
Sin1F(float a)
{
    // NOTE(tbt): range reduction
    int k = a*(1.0f / (2*Pi));
    a = a - k*(2*Pi);
    a = Min1F(a, Pi - a);
    a = Max1F(a, -Pi - a);
    a = Min1F(a, Pi - a);
    
    float result = 0.0f;
    
    float  a1 = a;
    float  a2 = a1*a1;
    float  a4 = a2*a2;
    float  a5 = a1*a4;
    float  a9 = a4*a5;
    float a13 = a9*a4;
    
    float term_1 =  a1*(1.0f - a2 /  6.0f);
    float term_2 =  a5*(1.0f - a2 /  42.0f) / 120.0f;
    float term_3 =  a9*(1.0f - a2 / 110.0f) / 362880.0f;
    float term_4 = a13*(1.0f - a2 / 225.0f) / 6227020800.0f;
    
    result += term_4;
    result += term_3;
    result += term_2;
    result += term_1;
    
    return result;
}

Function float
Cos1F(float a)
{
    return Sin1F(a + 0.5*Pi);
}

Function float
Tan1F(float a)
{
    return Sin1F(a) / Cos1F(a);
}

Function float
ACos1F(float a)
{
    float x = -0.939115566365855;
    float y =  0.9217841528914573;
    float z = -1.2845906244690837;
    float w =  0.295624144969963174;
    float result = Pi / 2.0f + (x*a + y*a*a*a) / (1.0f + z*a*a + w*a*a*a*a);
    return result;
}

Function float
Mod1F(float a, float b)
{
    double _a = a;
    double _b = b;
    double result = a - Floor1F(a / b)*b;
    
    return (float)result;
}

//~NOTE(tbt): float vectors

Function V4F
U4F(float a)
{
    V4F result =
    {
        .x = a,
        .y = a,
        .z = a,
        .w = a,
    };
    return result;
}

Function V4F
Add4F(V4F a, V4F b)
{
    V4F result =
    {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
        .w = a.w + b.w,
    };
    return result;
}

Function V4F
Sub4F(V4F a, V4F b)
{
    V4F result =
    {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
        .w = a.w - b.w,
    };
    return result;
}

Function V4F
Mul4F(V4F a, V4F b)
{
    V4F result =
    {
        .x = a.x*b.x,
        .y = a.y*b.y,
        .z = a.z*b.z,
        .w = a.w*b.w,
    };
    return result;
}

Function V4F
Div4F(V4F a, V4F b)
{
    V4F result =
    {
        .x = a.x / b.x,
        .y = a.y / b.y,
        .z = a.z / b.z,
        .w = a.w / b.w,
    };
    return result;
}

Function float
Dot4F(V4F a, V4F b)
{
    float result;
#if Build_UseSSE3
    __m128 _a = _mm_load_ps(a.elements);
    __m128 _b = _mm_load_ps(b.elements);
    __m128 mul_res = _mm_mul_ps(_a, _b);
    __m128 shuf_reg = _mm_movehdup_ps(mul_res);
    __m128 sums_reg = _mm_add_ps(mul_res, shuf_reg);
    shuf_reg = _mm_movehl_ps(shuf_reg, sums_reg);
    sums_reg = _mm_add_ss(sums_reg, shuf_reg);
    result = _mm_cvtss_f32(sums_reg);
#else
    result = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w);
#endif
    return result;
}

Function V4F
Scale4F(V4F a, float b)
{
    V4F _b = { b, b, b, b };
    return Mul4F(a, _b);
}

Function float
LengthSquared4F(V4F a)
{
    return a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w;
}

Function float
Length4F(V4F a)
{
    return Sqrt1F(LengthSquared4F(a));
}

Function V4F
Normalise4F(V4F a)
{
    float one_over_length = ReciprocalSqrt1F(LengthSquared4F(a));
    return Scale4F(a, one_over_length);
}

Function V4F
Mins4F(V4F a, V4F b)
{
    V4F result =
    {
        .x = Min1F(a.x, b.x),
        .y = Min1F(a.y, b.y),
        .z = Min1F(a.z, b.z),
        .w = Min1F(a.w, b.w),
    };
    return result;
}

Function V4F
Maxs4F(V4F a, V4F b)
{
    V4F result =
    {
        .x = Max1F(a.x, b.x),
        .y = Max1F(a.y, b.y),
        .z = Max1F(a.z, b.z),
        .w = Max1F(a.w, b.w),
    };
    return result;
}

//-

Function V3F
U3F(float a)
{
    V3F result =
    {
        .x = a,
        .y = a,
        .z = a,
    };
    return result;
}

Function V3F
Add3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
    };
    return result;
}

Function V3F
Sub3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
    };
    return result;
}

Function V3F
Mul3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = a.x*b.x,
        .y = a.y*b.y,
        .z = a.z*b.z,
    };
    return result;
}

Function V3F
Div3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = a.x / b.x,
        .y = a.y / b.y,
        .z = a.z / b.z,
    };
    return result;
}

Function float
Dot3F(V3F a, V3F b)
{
    V4F _a = { a.x, a.y, a.z };
    V4F _b = { b.x, b.y, b.z };
    return Dot4F(_a, _b);
}

Function V3F
Cross3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = a.y*b.z - a.z*b.y,
        .y = a.z*b.x - a.x*b.z,
        .z = a.x*b.y - a.y*b.x,
    };
    return result;
}

Function V3F
Scale3F(V3F a, float b)
{
    V3F _b = { b, b, b };
    return Mul3F(a, _b);
}

Function float
LengthSquared3F(V3F a)
{
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

Function float
Length3F(V3F a)
{
    return Sqrt1F(LengthSquared3F(a));
}

Function V3F
Normalise3F(V3F a)
{
    float one_over_length = ReciprocalSqrt1F(LengthSquared3F(a));
    return Scale3F(a, one_over_length);
}

Function V3F
Mins3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = Min1F(a.x, b.x),
        .y = Min1F(a.y, b.y),
        .z = Min1F(a.z, b.z),
    };
    return result;
}

Function V3F
Maxs3F(V3F a, V3F b)
{
    V3F result =
    {
        .x = Max1F(a.x, b.x),
        .y = Max1F(a.y, b.y),
        .z = Max1F(a.z, b.z),
    };
    return result;
}

//-

Function V2F
U2F(float a)
{
    V2F result =
    {
        .x = a,
        .y = a,
    };
    return result;
}

Function V2F
Add2F(V2F a, V2F b)
{
    V2F result =
    {
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
    return result;
}

Function V2F
Sub2F(V2F a, V2F b)
{
    V2F result =
    {
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
    return result;
}

Function V2F
Mul2F(V2F a, V2F b)
{
    V2F result =
    {
        .x = a.x*b.x,
        .y = a.y*b.y,
    };
    return result;
}

Function V2F
Div2F(V2F a, V2F b)
{
    V2F result =
    {
        .x = a.x / b.x,
        .y = a.y / b.y,
    };
    return result;
}

Function float
Dot2F(V2F a, V2F b)
{
    V4F _a = { a.x, a.y };
    V4F _b = { b.x, b.y };
    return Dot4F(_a, _b);
}

Function V2F
Scale2F(V2F a, float b)
{
    V2F _b = { b, b };
    return Mul2F(a, _b);
}

Function float
LengthSquared2F(V2F a)
{
    return a.x*a.x + a.y*a.y;
}

Function float
Length2F(V2F a)
{
    return Sqrt1F(LengthSquared2F(a));
}

Function V2F
Normalise2F(V2F a)
{
    float one_over_length = ReciprocalSqrt1F(LengthSquared2F(a));
    return Scale2F(a, one_over_length);
}


Function V2F
Mins2F(V2F a, V2F b)
{
    V2F result =
    {
        .x = Min1F(a.x, b.x),
        .y = Min1F(a.y, b.y),
    };
    return result;
}

Function V2F
Maxs2F(V2F a, V2F b)
{
    V2F result =
    {
        .x = Max1F(a.x, b.x),
        .y = Max1F(a.y, b.y),
    };
    return result;
}

//~NOTE(tbt): matrices

Function M4x4F
InitialiseDiagonal4x4F(float diag)
{
    M4x4F result = 
    {
        {
            { diag, 0.0f, 0.0f, 0.0f },
            { 0.0f, diag, 0.0f, 0.0f },
            { 0.0f, 0.0f, diag, 0.0f },
            { 0.0f, 0.0f, 0.0f, diag },
        },
    };
    return result;
}

#if Build_UseSSE2
typedef struct
{
    __m128 rows[4];
} SSE_M4x4F;

Function SSE_M4x4F
SSEM4x4FFromM4x4F_(M4x4F a)
{
    SSE_M4x4F result;
    result.rows[0] = _mm_load_ps(&a.elements[0][0]);
    result.rows[1] = _mm_load_ps(&a.elements[1][0]);
    result.rows[2] = _mm_load_ps(&a.elements[2][0]);
    result.rows[3] = _mm_load_ps(&a.elements[3][0]);
    return result;
}

Function M4x4F
M4x4FFromSSEM4x4F_(SSE_M4x4F a)
{
    M4x4F result;
    _mm_store_ps(&result.elements[0][0], a.rows[0]);
    _mm_store_ps(&result.elements[1][0], a.rows[1]);
    _mm_store_ps(&result.elements[2][0], a.rows[2]);
    _mm_store_ps(&result.elements[3][0], a.rows[3]);
    return result;
}

Function __m128
LinearCombine4x4F_(const __m128 *a,
                   const SSE_M4x4F *b)
{
    __m128 result;
    result = _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x00), b->rows[0]);
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x55), b->rows[1]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x55), b->rows[2]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x55), b->rows[3]));
    return result;
}

#endif

Function M4x4F
Mul4x4F(M4x4F a, M4x4F b)
{
    M4x4F result = {0};
#if Build_UseSSE2
    SSE_M4x4F _result;
    SSE_M4x4F _a = SSEM4x4FFromM4x4F_(a);
    SSE_M4x4F _b = SSEM4x4FFromM4x4F_(b);
    _result.rows[0] = LinearCombine4x4F_(&_a.rows[0], &_b);
    _result.rows[1] = LinearCombine4x4F_(&_a.rows[1], &_b);
    _result.rows[2] = LinearCombine4x4F_(&_a.rows[2], &_b);
    _result.rows[3] = LinearCombine4x4F_(&_a.rows[3], &_b);
    result = M4x4FFromSSEM4x4F_(_result);
#else
    for(int j = 0; j < 4; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            result.elements[i][j] = (a.elements[0][j]*b.elements[i][0] +
                                     a.elements[1][j]*b.elements[i][1] +
                                     a.elements[2][j]*b.elements[i][2] +
                                     a.elements[3][j]*b.elements[i][3]);
        }
    }
#endif
    return result;
}

Function M4x4F
PerspectiveMake4x4f(float fov,
                    float aspect_ratio,
                    float near, float far)
{
    M4x4F result = {0};
    float tan_half_theta = Tan1F(fov*(Pi/360.0f));
    result.elements[0][0] = 1.0f / tan_half_theta;
    result.elements[1][1] = aspect_ratio / tan_half_theta;
    result.elements[3][2] = -1.0f;
    result.elements[2][2] = (near + far) / (near - far);
    result.elements[2][3] = (2.0f*near*far) / (near - far);
    result.elements[3][3] = 0.0f;
    return result;
}

Function M4x4F
OrthoMake4x4F(float left, float right,
              float top, float bottom,
              float near, float far)
{
    M4x4F result = {0};
    result.elements[0][0] = +2.0f / (right - left);
    result.elements[1][1] = +2.0f / (top - bottom);
    result.elements[2][2] = -2.0f / (far - near);
    result.elements[3][3] = 1.0f;
    result.elements[3][0] = -((right + left) / (right - left));
    result.elements[3][1] = -((top + bottom) / (top - bottom));
    result.elements[3][2] = -((far + near) / (far - near));
    return result;
}

Function M4x4F
LookAtMake4x4F(V3F eye, V3F centre, V3F up)
{
    M4x4F result;
    
    V3F f = Normalise3F(Sub3F(centre, eye));
    V3F s = Normalise3F(Cross3F(f, up));
    V3F u = Cross3F(s, f);
    
    result.elements[0][0] = s.x;
    result.elements[0][1] = u.x;
    result.elements[0][2] = -f.x;
    result.elements[0][3] = 0.0f;
    
    result.elements[1][0] = s.y;
    result.elements[1][1] = u.y;
    result.elements[1][2] = -f.y;
    result.elements[1][3] = 0.0f;
    
    result.elements[2][0] = s.z;
    result.elements[2][1] = u.z;
    result.elements[2][2] = -f.z;
    result.elements[2][3] = 0.0f;
    
    result.elements[3][0] = -Dot3F(s, eye);
    result.elements[3][1] = -Dot3F(u, eye);
    result.elements[3][2] = Dot3F(f, eye);
    result.elements[3][3] = 1.0f;
    
    return result;
}

Function M4x4F
TranslationMake4x4F(V3F translation)
{
    M4x4F result = InitialiseDiagonal4x4F(1.0f);
    result.elements[3][0] = translation.x;
    result.elements[3][1] = translation.y;
    result.elements[3][2] = translation.z;
    return result;
}

Function M4x4F
ScaleMake4x4F(V3F scale)
{
    M4x4F result = InitialiseDiagonal4x4F(1.0f);
    result.elements[0][0] = scale.x;
    result.elements[1][1] = scale.y;
    result.elements[2][2] = scale.z;
    return result;
}

Function V4F
Transform4F(V4F a, M4x4F b)
{
    V4F result =
    {
        .x = b.elements[0][0]*a.x + b.elements[0][1]*a.y + b.elements[0][2]*a.z + b.elements[0][3]*a.w,
        .y = b.elements[1][0]*a.x + b.elements[1][1]*a.y + b.elements[1][2]*a.z + b.elements[1][3]*a.w,
        .z = b.elements[2][0]*a.x + b.elements[2][1]*a.y + b.elements[2][2]*a.z + b.elements[2][3]*a.w,
        .w = b.elements[3][0]*a.x + b.elements[3][1]*a.y + b.elements[3][2]*a.z + b.elements[3][3]*a.w,
    };
    return result;
}

//~NOTE(tbt): single integers

Function int
InterpolateLinear1I(int a, int b,
                    unsigned char t) // NOTE(tbt): (0 <= t <= 255)
{
    return ((t*(b - a)) >> 8) + a;
}

Function int
Min1I(int a, int b)
{
    if(a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

Function int
Max1I(int a, int b)
{
    if(a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

Function int
Clamp1I(int a, int min, int max)
{
    return Max1I(min, Min1I(max, a));
}

Function int
Abs1I(int a)
{
    return (a < 0) ? -a : a;
}

Function int
RotL1I(int a, int b)
{
    return (a << b) | (a >> (32 - b));
}

Function int
RotR1I(int a, int b)
{
    return (a >> b) | (a << (32 - b));
}

Function int
Normalise1I(int a)
{
    int result = 0;
    if(a < 0)
    {
        result = -1;
    }
    else if(a > 0)
    {
        result = +1;
    }
    return result;
}

//~NOTE(tbt): integer vectors

Function V4I
U4I(int a)
{
    V4I result =
    {
        .x = a,
        .y = a,
        .z = a,
        .w = a,
    };
    return result;
}

Function V4I
Add4I(V4I a, V4I b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

Function V4I
Sub4I(V4I a, V4I b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

Function V4I
Mul4I(V4I a, V4I b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
    return a;
}

Function V4I
Div4I(V4I a, V4I b)
{
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    a.w /= b.w;
    return a;
}

Function int
Dot4I(V4I a, V4I b)
{
    int result = 0;
    result += a.x*b.x;
    result += a.y*b.y;
    result += a.z*b.z;
    result += a.w*b.w;
    return result;
}

Function int
LengthSquared4I(V4I a)
{
    int result = a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w;
    return result;
}

Function int
Length4I(V4I a)
{
    int result = Sqrt1F(LengthSquared4I(a)) + 0.5f;
    return result;
}

//-

Function V3I
U3I(int a)
{
    V3I result =
    {
        .x = a,
        .y = a,
        .z = a,
    };
    return result;
}

Function V3I
Add3I(V3I a, V3I b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

Function V3I
Sub3I(V3I a, V3I b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

Function V3I
Mul3I(V3I a, V3I b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    return a;
}

Function V3I
Div3I(V3I a, V3I b)
{
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    return a;
}

Function int
Dot3I(V3I a, V3I b)
{
    int result = 0;
    result += a.x*b.x;
    result += a.y*b.y;
    result += a.z*b.z;
    return result;
}

Function V3I
Cross3i(V3I a, V3I b)
{
    V3I result =
    {
        .x = a.y*b.z - a.z*b.y,
        .y = a.z*b.x - a.x*b.z,
        .z = a.x*b.y - a.y*b.x,
    };
    return result;
}

Function int
LengthSquared3I(V3I a)
{
    int result = a.x*a.x + a.y*a.y + a.z*a.z;
    return result;
}

Function int
Length3I(V3I a)
{
    int result = Sqrt1F(LengthSquared3I(a)) + 0.5f;
    return result;
}

//-

Function V2I
U2I(int a)
{
    V2I result =
    {
        .x = a,
        .y = a,
    };
    return result;
}

Function V2I
Add2I(V2I a, V2I b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

Function V2I
Sub2I(V2I a, V2I b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

Function V2I
Mul2I(V2I a, V2I b)
{
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

Function V2I
Div2I(V2I a, V2I b)
{
    a.x /= b.x;
    a.y /= b.y;
    return a;
}

Function int
Dot2I(V2I a, V2I b)
{
    int result = 0;
    result += a.x*b.x;
    result += a.y*b.y;
    return result;
}

Function int
LengthSquared2I(V2I a)
{
    int result = a.x*a.x + a.y*a.y;
    return result;
}

Function int
Length2I(V2I a)
{
    int result = Sqrt1F(LengthSquared2I(a)) + 0.5f;
    return result;
}

//~NOTE(tbt): single unsigned integers

Function size_t
Min1U(size_t a, size_t b)
{
    if(a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

Function size_t
Max1U(size_t a, size_t b)
{
    if(a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

Function size_t
Clamp1U(size_t a, size_t min, size_t max)
{
    return Max1U(min, Min1U(max, a));
}

//~NOTE(tbt): intervals

Function Bool
IntervalHasValue1F(I1F a, float b)
{
    Bool result = (a.min < b && b < a.max);
    return result;
}

Function float
Centre1F(I1F a)
{
    return (a.min + a.max) / 2.0f;
}

Function I2F
RectMake2F(V2F pos, V2F dimensions)
{
    V2F max = Add2F(pos, dimensions);
    I2F result =
    {
        .min = Mins2F(pos, max),
        .max = Maxs2F(pos, max),
    };
    return result;
}

Function I2F
Expand2F(I2F a, V2F b)
{
    I2F result =
    {
        .min = Sub2F(a.min, b),
        .max = Add2F(a.max, b),
    };
    return result;
}

Function Bool
SAT2F(I2F a, I2F b)
{
    if(a.max.x < b.min.x || a.min.x >= b.max.x) { return False; }
    if(a.max.y < b.min.y || a.min.y >= b.max.y) { return False; }
    return True;
}

Function Bool
IntervalHasValue2F(I2F a, V2F b)
{
    if(b.x < a.min.x || b.x >= a.max.x) { return False; }
    if(b.y < a.min.y || b.y >= a.max.y) { return False; }
    return True;
}

Function V2F
Centre2F(I2F a)
{
    V2F result =
    {
        .x = Centre1F(I1F(a.min.x, a.max.x)),
        .y = Centre1F(I1F(a.min.y, a.max.y)),
    };
    return result;
}

Function I2F
Intersection2F(I2F a, I2F b)
{
    I2F result;
    result.min.x = Max1F(a.min.x, b.min.x);
    result.min.y = Max1F(a.min.y, b.min.y);
    result.max.x = Min1F(a.max.x, b.max.x);
    result.max.y = Min1F(a.max.y, b.max.y);
    return result;
}

Function V2F
Dimensions2F(I2F a)
{
    V2F result =
    {
        .x = a.max.x - a.min.x,
        .y = a.max.y - a.min.y,
    };
    return result;
}

//~NOTE(tbt): colours

Function Pixel
InterpolateLinearPixel(Pixel a, Pixel b, unsigned char t)
{
    Pixel result;
    if(0 == t)
    {
        result = a;
    }
    else if(255 == t)
    {
        result = b;
    }
    else
    {
        result.b = ((t*(b.b - a.b)) >> 8) + a.b;
        result.g = ((t*(b.g - a.g)) >> 8) + a.g;
        result.r = ((t*(b.r - a.r)) >> 8) + a.r;
    }
    return result;
}

Function V4F
HSVFromRGB (V4F col)
{
    V4F result = {0};
    
    V4F _col =
    {
        .r = col.r / col.a,
        .g = col.g / col.a,
        .b = col.b / col.a,
        .a = 1.0f,
    };
    
    float max = Max1F(Max1F(_col.r, _col.g), _col.b);
    float min = Min1F(Min1F(_col.r, _col.g), _col.b);
    float delta = max - min;
    
    if(delta > 0.0f)
    {
        if(max == _col.r)
        {
            result.x = 60.0f*(Mod1F(((_col.g - _col.b) / delta), 6.0f));
        }
        else if(max == _col.g)
        {
            result.x = 60.0f*(((_col.b - _col.r) / delta) + 2.0f);
        }
        else if(max == _col.b)
        {
            result.x = 60.0f*(((_col.r - _col.g) / delta) + 4.0f);
        }
        
        if(max > 0)
        {
            result.y = delta / max;
        }
        else
        {
            result.y = 0.0f;
        }
        
        result.z = max;
    }
    else
    {
        result.x = 0.0f;
        result.y = 0.0f;
        result.z = max;
    }
    
    if(result.x < 0.0f)
    {
        result.x = 360.0f + result.x;
    }
    
    result.a = col.a;
    result.x /= 360.0f;
    
    return result;
}

Function V4F
RGBFromHSV (V4F col)
{
    V4F result = {0};
    
    float chroma = col.z*col.y;
    float h = Mod1F(col.x*6.0f, 6.0f);
    float x = chroma*(1.0f - Abs1F(Mod1F(h, 2.0f) - 1.0f));
    float m = col.z - chroma;
    
    if(0.0f <= h && h < 1.0f)
    {
        result = V4F(chroma, x, 0.0f, 0.0f);
    }
    else if(1.0f <= h && h < 2.0f)
    {
        result = V4F(x, chroma, 0.0f, 0.0f);
    }
    else if(2.0f <= h && h < 3.0f)
    {
        result = V4F(0.0f, chroma, x, 0.0f);
    }
    else if(3.0f <= h && h < 4.0f)
    {
        result = V4F(0.0f, x, chroma, 0.0f);
    }
    else if(4.0f <= h && h < 5.0f)
    {
        result = V4F(x, 0.0f, chroma, 0.0f);
    }
    else if(5.0f <= h && h < 6.0f)
    {
        result = V4F(chroma, 0.0f, x, 0.0f);
    }
    
    result = Add4F(result, V4F(m, m, m, col.a));
    result = Mul4F(result, V4F(result.a, result.a, result.a, 1.0f));
    return result;
}
