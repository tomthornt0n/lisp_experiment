#undef STB_SPRINTF_IMPLEMENTATION
#include "external/stb_sprintf.h"

#include "external/zlib/zlib.h"

//~NOTE(tbt): string types

//-NOTE(tbt): utf8 ecnoded strings
typedef struct
{
    char *buffer;
    size_t len;
} S8;
#define S8(S) ((S8){ S, sizeof(S) - 1 })
#define S8Initialiser(S) { .buffer = S, .len = sizeof(S) - 1, }

//-NOTE(tbt): utf16 ecnoded strings
typedef struct
{
    wchar_t *buffer;
    size_t len;
} S16;
#define S16(S) ((S16){ WidenString(S), (sizeof(WidenString(S)) - 2) / 2 })

//-NOTE(tbt): utf32 ecnoded strings
typedef struct
{
    unsigned int *buffer;
    size_t len;
} S32;

//~NOTE(tbt): fmt string helpers

#define FmtS8(S) (int)(S).len, (S).buffer        // NOTE(tbt): S8   --> "%.*s"

#define FmtBool(B) ((!!(B)) ? "True" : "False")  // NOTE(tbt): Bool --> "%s"

#define FmtV4F(V) (V).x, (V).y, (V).z, (V).w     // NOTE(tbt): V4F  --> "%f%f%f%f"
#define FmtV3F(V) (V).x, (V).y, (V).z            // NOTE(tbt): V3F  --> "%f%f%f"
#define FmtV2F(V) (V).x, (V).y                   // NOTE(tbt): V2F  --> "%f%f"

#define FmtV4I(V) (V).x, (V).y, (V).z, (V).w     // NOTE(tbt): V4I  --> "%d%d%d%d"
#define FmtV3I(V) (V).x, (V).y, (V).z            // NOTE(tbt): V3I  --> "%d%d%d"
#define FmtV2I(V) (V).x, (V).y                   // NOTE(tbt): V2I  --> "%d%d"

//~NOTE(tbt): character utilites

Function Bool CharIsSymbol       (unsigned int c);
Function Bool CharIsSpace        (unsigned int c);
Function Bool CharIsNumber       (unsigned int c);
Function Bool CharIsUppercase    (unsigned int c);
Function Bool CharIsLowercase    (unsigned int c);
Function Bool CharIsLetter       (unsigned int c);
Function Bool CharIsAlphanumeric (unsigned int c);
Function Bool CharIsPrintable    (unsigned int c);

Function unsigned int CharLowerFromUpper (unsigned int c);
Function unsigned int CharUpperFromLower (unsigned int c);

//~NOTE(tbt): c-string helpers

Function size_t CStringCalculateUTF8Length  (char *cstring);
Function S8     CStringAsS8                 (char *cstring);
Function size_t CStringCalculateUTF16Length (wchar_t *cstring);
Function S16    CStringAsS16                (wchar_t *cstring);

//~NOTE(tbt): unicode conversions

typedef struct
{
    unsigned int codepoint;
    int advance;
} UTFConsume;

Function Bool UTF8IsContinuationByte (S8 string, int index);

// NOTE(tbt): from utf8
Function UTFConsume CodepointFromUTF8 (S8 string, int index);
Function S16        S16FromS8         (M_Arena *arena, S8 string);
Function S32        S32FromS8         (M_Arena *arena, S8 string);

// NOTE(tbt): from utf16
Function UTFConsume CodepointFromUTF16 (S16 string, int index);
Function S8         S8FromS16          (M_Arena *arena, S16 string);
Function S32        S32FromS16         (M_Arena *arena, S16 string);

// NOTE(tbt): from utf32
Function S8  UTF8FromCodepoint  (M_Arena *arena, unsigned int codepoint);
Function S16 UTF16FromCodepoint (M_Arena *arena, unsigned int codepoint);
Function S8  S8FromS32          (M_Arena *arena, S32 string);
Function S16 S16FromS32         (M_Arena *arena, S32 string);

//~NOTE(tbt): string operations

typedef enum
{
    MatchFlags_Exact           = 0,
    MatchFlags_RightSideSloppy = Bit(0), // NOTE(tbt): if the lengths are not equal, compare up to the length of the shortest string
    MatchFlags_CaseInsensitive = Bit(1),
} MatchFlags;

Function S8       S8Clone                  (M_Arena *arena, S8 string);
Function S8       S8CloneFL                (M_FreeList *free_list, S8 string);
Function S8       S8Truncate               (S8 string, size_t len);
Function S8       S8LFFromCRLF             (M_Arena *arena, S8 string);
Function S8       S8FromFmtV               (M_Arena *arena, char *fmt, va_list args);
Function S8       S8FromFmt                (M_Arena *arena, char *fmt, ...);
Function S8       S8Strip                  (M_Arena *arena, S8 a, int b);
Function S8       S8PrefixGet              (S8 string, size_t len);
Function S8       S8SuffixGet              (S8 string, size_t len);
Function S8       S8Advance                (S8 *string, size_t n);
Function Bool     S8Consume                (S8 *string, S8 consume);
Function size_t   S8ByteIndexFromCharIndex (S8 string, size_t char_index);
Function size_t   S8CharIndexFromByteIndex (S8 string, size_t byte_index);
Function uint32_t S8Hash                   (S8 string);
Function Bool     S8Match                  (S8 a, S8 b, MatchFlags flags);
Function S8       S8Substring              (S8 h, S8 n, MatchFlags flags);
Function Bool     S8HasSubstring           (S8 h, S8 n, MatchFlags flags);
Function double   S8Parse1F                (S8 string);

Function S16      S16Clone                 (M_Arena *arena, S16 string);
Function S16      S16CloneFL               (M_FreeList *free_list, S16 string);
Function uint32_t S16Hash                  (S16 string);
Function Bool     S16Match                 (S16 a, S16 b, MatchFlags flags);



//~NOTE(tbt): filename helpers

#if Build_OSWindows
# define DirectorySeparatorStr "\\"
# define DirectorySeparatorChr '\\'
#else
# define DirectorySeparatorStr "/"
# define DirectorySeparatorChr '/'
#endif
Function Bool     FilenameHasTrailingSlash    (S8 filename);
Function S8       ExtensionFromFilename       (S8 filename);
Function S8       FilenamePush                (M_Arena *arena, S8 filename, S8 string);
Function S8       FilenamePop                 (S8 filename);
Function S8       FilenameLast                (S8 filename);
Function Bool     FilenameIsChild             (S8 parent, S8 filename);

//~NOTE(tbt): misc.

Function Bool     S8IsWordBoundary            (S8 string, size_t index);

//~NOTE(tbt): string lists

typedef struct S8Node S8Node;
struct S8Node
{
    S8 string;
    S8Node *next;
};

typedef struct
{
    S8Node *first;
    S8Node *last;
    size_t count;
} S8List;

typedef struct
{
    Bool has;
    size_t i;
} S8ListFind;
Function S8ListFind S8ListIndexFromS8            (S8List list, S8 string, MatchFlags flags);

typedef struct
{
    S8 prefix;    // NOTE(tbt): concatenated to the start of each node
    S8 delimiter; // NOTE(tbt): added between every node
    S8 suffix;    // NOTE(tbt): concatenated to the end of each node
} S8ListJoinFormat;
#define S8ListJoinFormat(...) ((S8ListJoinFormat){ __VA_ARGS__ })

// NOTE(tbt): clones the string to the arena and allocates a new node in the arena
Function S8Node    *S8ListPush                   (M_Arena *arena, S8List *list, S8 string);
Function S8Node    *S8ListAppend                 (M_Arena *arena, S8List *list, S8 string);
Function void       S8ListConcatenate            (M_Arena *arena, S8List *a, S8List b); // NOTE(tbt): DOES NOT CLONE A

// NOTE(tbt): simply inserts the passed node into the list
Function void       S8ListPushExplicit           (S8List *list, S8Node *string);
Function void       S8ListAppendExplicit         (S8List *list, S8Node *string);

Function S8List     S8ListClone                  (M_Arena *arena, S8List list);
Function S8         S8ListS8FromIndex            (S8List list, size_t index);
Function Bool       S8ListHasS8                  (S8List list, S8 string, MatchFlags flags);
Function S8         S8ListJoin                   (M_Arena *arena, S8List list);
Function S8         S8ListJoinFormated           (M_Arena *arena, S8List list, S8ListJoinFormat join);
Function S8Node    *S8ListRemoveFirstOccurenceOf (S8List *list, S8 string, MatchFlags flags);
Function void       S8ListRemoveAllOccurencesOf  (S8List *list, S8 string, MatchFlags flags);
Function void       S8ListRemoveExplicit         (S8List *list, S8Node *string);

#define S8ListForEach(L, V) S8Node *(V) = (L).first; 0 != (V); (V) = (V)->next

Function void S8ListRecalculate (S8List *list);

//~NOTE(tbt): compression

Function S8 S8Deflate (M_Arena *arena, S8 string);
Function S8 S8Inflate (M_Arena *arena, S8 string);

//~NOTE(tbt): serialisation/deserialisation

// NOTE(tbt): read their respective types from a binary string and advance the input string

#define S8ReadType(T, S) (((S).len >= sizeof(T)) ? (*((T *)((S).buffer))) : (T){0})
Function uint8_t  S8DeserialiseU8  (S8 *data);
Function uint16_t S8DeserialiseU16 (S8 *data);
Function uint32_t S8DeserialiseU32 (S8 *data);
Function uint64_t S8DeserialiseU64 (S8 *data);
Function int8_t   S8DeserialiseI8  (S8 *data);
Function int16_t  S8DeserialiseI16 (S8 *data);
Function int32_t  S8DeserialiseI32 (S8 *data);
Function int64_t  S8DeserialiseI64 (S8 *data);
Function float    S8Deserialise1F  (S8 *data);
Function V2F      S8Deserialise2F  (S8 *data);
Function V3F      S8Deserialise3F  (S8 *data);
Function V4F      S8Deserialise4F  (S8 *data);

Function void S8SerialiseBytes (S8 *data, void *src, size_t bytes);
#define S8SerialiseType(T, S, V) Statement( if((S)->len >= sizeof(T)) { *((T *)((S)->buffer)) = (V); S8Advance((S), sizeof(T)); } )
#define S8SerialiseStruct(S, V) Statement( S8SerialiseBytes((S), &(V), sizeof(V)); )
