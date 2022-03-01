
//~NOTE(tbt): avoid ambiguous 'static' keyword

// NOTE(tbt): WARING - these are #undef'ed and then re-#define'd when
//                     including <windows.h> to avoid conflicts. Only changing
//                     them here might not do what you think

#define Function static
#define Global static
#define Persist static

//~NOTE(tbt): preprocessor helpers

#define Glue_(A, B) A ## B
#define Glue(A, B) Glue_(A, B)

#define WidenString(S) Glue_(L, S)

#define Stringify_(A) #A
#define Stringify(A) Stringify_(A)
#define WideStringify(A) WidenString(Stringify(A))

#define Statement(S) do { S } while(False)

//~NOTE(tbt): size helpers

#define Bytes(N)      Glue_(N, LLU)
#define Kilobytes(N)  (1000LLU * (uint64_t)Bytes(N))
#define Megabytes(N)  (1000LLU * (uint64_t)Kilobytes(N))
#define Gigabytes(N)  (1000LLU * (uint64_t)Megabytes(N))
#define Terabytes(N)  (1000LLU * (uint64_t)Gigabytes(N))

//~NOTE(tbt): useful constants

#define Pi (3.1415926535897f)

Global union
{
    uint32_t u;
    float f;
} Infinity_ =
{
    .u = 0x7F800000,
};
#define Infinity (Infinity_.f)

Global union
{
    uint32_t u;
    float f;
} NegativeInfinity_ =
{
    .u = 0xFF800000,
};
#define NegativeInfinity (NegativeInfinity_.f)

Global union
{
	uint8_t u[sizeof(float)];
	float f;
} NaN_ =
{
	.u = { 0, 0, 0xc0, 0x7f, },
};
#define NaN (NaN_.f)


//~NOTE(tbt): misc helpers

#define ArrayCount(A) (sizeof(A) / sizeof((A)[0]))

#define Bit(N) (1 << N)

#define ApplicationNameString Stringify(ApplicationName)
#define ApplicationNameWString WideStringify(ApplicationName)

#define DeferLoop(BEGIN, END) for(Bool _i = ((BEGIN), True); True == _i; ((END), _i = False))

#define Eq(A, B) (sizeof(A) == sizeof(B) && M_Compare(&(A), &(B), sizeof(A)))
#define SwapStruct(A, B) Swap(&(A), &(B), Min(sizeof(A), sizeof(B)))

#if Build_CompilerMSVC
# pragma section(".roglob", read)
# define ReadOnly __declspec(allocate(".roglob"))
#else
# define ReadOnly const
#endif

//#define case(C) case(C): case_ ## C

Function int  WrapToBounds (int a, float min, float max);
Function void Swap         (unsigned char *a, unsigned char *b, int64_t bytes_per_item);

Function float  MinInF (float elements[], size_t n);
Function float  MaxInF (float elements[], size_t n);
Function int    MinInI (int elements[], size_t n);
Function int    MaxInI (int elements[], size_t n);
Function size_t MinInU (size_t elements[], size_t n);
Function size_t MaxInU (size_t elements[], size_t n);

//~NOTE(tbt): type generic math helpers
#define Min(A, B) ((A) < (B) ? (A) : (B))
#define Max(A, B) ((A) > (B) ? (A) : (B))
#define Clamp(X, A, B) (Max(A, Min(B, X)))
#define Abs(A) ((A) < 0 ? -(A) : (A))

//~NOTE(tbt): pointer arithmetic

#define IntFromPtr(P) ((uintptr_t)((unsigned char *)(P) - (unsigned char *)0))
#define PtrFromInt(N) ((void *)((unsigned char *)0 + (N)))

#define Member(T, M) (&((T *)0)->M)
#define OffsetOf(T, M) IntFromPtr(Member(T, M))

//~NOTE(tbt): asserts

#if Build_OSWindows
#  define AssertBreak_() __debugbreak()
#else
#  define AssertBreak_() volatile int i = *((int *)0)
#endif

typedef void AssertLogHook(char *title, char *msg);
Global AssertLogHook *assert_log = 0;
Function void Assert_(int c, char *msg);
# define Assert(C) Statement(Assert_((C), __FILE__ "(" Stringify(__LINE__) "): assertion failure: " #C);)


//~NOTE(tbt): misc types

typedef int Bool;
enum Bool
{
    False = 0,
    True = 1,
};

typedef unsigned int DataAccessFlags;
typedef enum
{
    DataAccessFlags_Read    = Bit(0),
    DataAccessFlags_Write   = Bit(1),
    DataAccessFlags_Execute = Bit(2),
} DataAccessFlags_ENUM;

//~NOTE(tbt): signed/unsigned encoding

Function int64_t  I64DecodeFromU64(uint64_t a);
Function uint64_t I64EncodeAsU64(int64_t a);
