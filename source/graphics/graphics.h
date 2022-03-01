
//~NOTE(tbt): libraries

// NOTE(tbt): stb rect pack (for stb truetype)

#define STBRP_ASSERT(A) Assert(A)
#define STBRP_SORT(B, N, W, C) Sort(B, N, W, C, 0)
#define STB_RECT_PACK_IMPLEMENTATION
#include "external/stb_rect_pack.h"

// NOTE(tbt): stb true type

#if Build_NoCRT
M_Arena r_arena_for_stb_truetype = {0};
# define STBTT_malloc(X, U) (M_ArenaPush(&r_arena_for_stb_truetype, X))
# define STBTT_free(X, U)   (0)
# define STBTT_ifloor(A)    ((int)Floor1F(A))
# define STBTT_iceil(A)     ((int)Ceil1F(A))
# define STBTT_sqrt(A)      (Sqrt1F(A))
# define STBTT_pow(A, B)    (Pow1F(A, B))
# define STBTT_fmod(A, B)   (Mod1F(A, B))
# define STBTT_cos(A)       (Cos1F(A))
# define STBTT_acos(A)      (ACos1F(A))
# define STBTT_fabs(A)      (Abs1F(A))
# define STBTT_assert(A)    Assert(A)
# define STBTT_strlen(A)    (CStringCalculateUTF8Length(A))
# define STBTT_memcpy       M_Copy
# define STBTT_memset       M_Set
#endif

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"

//~NOTE(tbt): headers

#include "graphics__input.h"
#include "graphics__events.h"
#include "graphics__window.h"
#include "graphics__renderer.h"
#include "graphics__app.h"
