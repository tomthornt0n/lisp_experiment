
//~NOTE(tbt): single floats

Function Bool  IsNaN1F                  (float a);
Function float Smoothstep1F             (float t);
Function float InterpolateLinear1F      (float a, float b, float t);
Function float InterpolateExponential1F (float a, float b, float t);
Function float InterpolateSmooth1F      (float a, float b, float t);
Function float Min1F                    (float a, float b);
Function float Max1F                    (float a, float b);
Function float Clamp1F                  (float a, float min, float max);
Function float Abs1F                    (float a);
Function float Pow1F                    (float a, float b);
Function float Round1F                  (float a);
Function float Floor1F                  (float a);
Function float Ceil1F                   (float a);
Function float Fract1F                  (float a);
Function float Sqrt1F                   (float a);
Function float ReciprocalSqrt1F         (float a);
Function float Sin1F                    (float a);
Function float Cos1F                    (float a);
Function float Tan1F                    (float a);
Function float ACos1F                   (float a);
Function float Mod1F                    (float a, float b);

//~NOTE(tbt): floating point vectors

typedef union
{
    struct
    {
        float x;
        float y;
    };
    float elements[2];
} V2F;
#define V2F(...) ((V2F){ __VA_ARGS__ })

typedef union
{
    struct
    {
        float x;
        float y;
        float z;
    };
    struct
    {
        float r;
        float g;
        float b;
    };
    float elements[3];
} V3F;
#define V3F(...) ((V3F){ __VA_ARGS__ })

typedef union
{
    struct
    {
        float x;
        float y;
        float z;
        float w;
    };
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
    float elements[4];
} V4F;
#define V4F(...) ((V4F){ __VA_ARGS__ })
#define Col(R, G, B, A) V4F((R)*(A), (G)*(A), (B)*(A), (A))

Function V4F   U4F             (float a);
Function V4F   Add4F           (V4F a, V4F b);
Function V4F   Sub4F           (V4F a, V4F b);
Function V4F   Mul4F           (V4F a, V4F b);
Function V4F   Div4F           (V4F a, V4F b);
Function float Dot4F           (V4F a, V4F b);
Function V4F   Scale4F         (V4F a, float b);
Function float LengthSquared4F (V4F a);
Function float Length4F        (V4F a);
Function V4F   Normalise4F     (V4F a);
Function V4F   Mins4F          (V4F a, V4F b);
Function V4F   Maxs4F          (V4F a, V4F b);

Function V3F   U3F             (float a);
Function V3F   Add3F           (V3F a, V3F b);
Function V3F   Sub3F           (V3F a, V3F b);
Function V3F   Mul3F           (V3F a, V3F b);
Function V3F   Div3F           (V3F a, V3F b);
Function float Dot3F           (V3F a, V3F b);
Function V3F   Scale3F         (V3F a, float b);
Function float LengthSquared3F (V3F a);
Function float Length3F        (V3F a);
Function V3F   Normalise3F     (V3F a);
Function V3F   Mins3F          (V3F a, V3F b);
Function V3F   Maxs3F          (V3F a, V3F b);
Function V3F   Cross3F         (V3F a, V3F b);

Function V2F   U2F             (float a);
Function V2F   Add2F           (V2F a, V2F b);
Function V2F   Sub2F           (V2F a, V2F b);
Function V2F   Mul2F           (V2F a, V2F b);
Function V2F   Div2F           (V2F a, V2F b);
Function float Dot2F           (V2F a, V2F b);
Function V2F   Scale2F         (V2F a, float b);
Function float LengthSquared2F (V2F a);
Function float Length2F        (V2F a);
Function V2F   Normalise2F     (V2F a);
Function V2F   Mins2F          (V2F a, V2F b);
Function V2F   Maxs2F          (V2F a, V2F b);

//~NOTE(tbt): matrices

typedef union
{
    struct
    {
        V4F r_0;
        V4F r_1;
        V4F r_2;
        V4F r_3;
    };
    float elements[4][4];
} M4x4F;

Function M4x4F InitialiseDiagonal4x4F (float diag);
Function M4x4F Mul4x4F                (M4x4F a, M4x4F b);
Function M4x4F PerspectiveMake4x4f    (float fov, float aspect_ratio, float near, float far);
Function M4x4F OrthoMake4x4F          (float left, float right, float top, float bottom, float near, float far);
Function M4x4F LookAtMake4x4F         (V3F eye, V3F centre, V3F up);
Function M4x4F TranslationMake4x4F    (V3F translation);
Function M4x4F ScaleMake4x4F          (V3F scale);
Function V4F   Transform4F            (V4F a, M4x4F b);

//~NOTE(tbt): single integers

Function int InterpolateLinear1I (int a, int b, unsigned char t);
Function int Min1I               (int a, int b);
Function int Max1I               (int a, int b);
Function int Clamp1I             (int a, int min, int max);
Function int Abs1I               (int a);
Function int RotL1I              (int a, int b);
Function int RotR1I              (int a, int b);
Function int Normalise1I         (int a);

//~NOTE(tbt): integer vectors

typedef union
{
    struct
    {
        int x;
        int y;
    };
    int elements[2];
} V2I;
#define V2I(...) ((V2I){ __VA_ARGS__ })

typedef union
{
    struct
    {
        int x;
        int y;
        int z;
    };
    int elements[3];
} V3I;
#define V3I(...) ((V3I){ __VA_ARGS__ })

typedef union
{
    struct
    {
        int x;
        int y;
        int z;
        int w;
    };
    int elements[4];
} V4I;
#define V4I(...) ((V4I){ __VA_ARGS__ })

Function V4I U4I             (int a);
Function V4I Add4I           (V4I a, V4I b);
Function V4I Sub4I           (V4I a, V4I b);
Function V4I Mul4I           (V4I a, V4I b);
Function V4I Div4I           (V4I a, V4I b);
Function int Dot4I           (V4I a, V4I b);
Function int LengthSquared4I (V4I a);
Function int Length4I        (V4I a);

Function V3I U3I     (int a);
Function V3I Add3I   (V3I a, V3I b);
Function V3I Sub3I   (V3I a, V3I b);
Function V3I Mul3I   (V3I a, V3I b);
Function V3I Div3I   (V3I a, V3I b);
Function int Dot3I   (V3I a, V3I b);
Function V3I Cross3I (V3I a, V3I b);
Function int LengthSquared3I (V3I a);
Function int Length3I        (V3I a);

Function V2I U2I     (int a);
Function V2I Add2I   (V2I a, V2I b);
Function V2I Sub2I   (V2I a, V2I b);
Function V2I Mul2I   (V2I a, V2I b);
Function V2I Div2I   (V2I a, V2I b);
Function int Dot2I   (V2I a, V2I b);
Function int LengthSquared2I (V2I a);
Function int Length2I        (V2I a);

//~NOTE(tbt): single unsigned integers

Function size_t Min1U   (size_t a, size_t b);
Function size_t Max1U   (size_t a, size_t b);
Function size_t Clamp1U (size_t a, size_t min, size_t max);

//~NOTE(tbt): intervals

typedef union
{
    struct
    {
        float min;
        float max;
    };
    float elements[2];
    V2F v;
} I1F;
#define I1F(...) ((I1F){ __VA_ARGS__ })

static Bool  IntervalHasValue1F (I1F a, float b);
static float Centre1F           (I1F a);

typedef union
{
    struct
    {
        size_t min;
        size_t max;
    };
    struct
    {
        size_t cursor;
        size_t mark;
    };
    size_t elements[2];
} I1U;
#define I1U(...) ((I1U){ __VA_ARGS__ })

typedef union
{
    struct
    {
        int min;
        int max;
    };
    int elements[2];
    V2I v;
} I1I;
#define I1I(...) ((I1I){ __VA_ARGS__ })

typedef union
{
    struct
    {
        V2F min;
        V2F max;
    };
    struct
    {
        V2F p0;
        V2F p1;
    };
    struct
    {
        float x0;
        float y0;
        float x1;
        float y1;
    };
    V2F points[2];
    float elements[4];
} I2F;
#define I2F(...) ((I2F){ __VA_ARGS__ })

Function I2F  RectMake2F         (V2F pos, V2F dimensions);
Function I2F  Expand2F           (I2F a, V2F b);
Function Bool SAT2F              (I2F a, I2F b);
Function Bool IntervalHasValue2F (I2F a, V2F b);
Function V2F  Centre2F           (I2F a);
Function I2F  Intersection2F     (I2F a, I2F b);
Function V2F  Dimensions2F       (I2F a);

//~NOTE(tbt): colours

typedef union
{
    uint32_t val;
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
} Pixel;
#define Pixel(...) ((Pixel){ __VA_ARGS__ })

Function V4F HSVFromRGB (V4F col);
Function V4F RGBFromHSV (V4F col);
