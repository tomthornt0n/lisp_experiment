#define STB_SPRINTF_IMPLEMENTATION
#include "external/stb_sprintf.h"

//~NOTE(tbt): character utilities

// TODO(tbt): make these work more generally and not just on ASCII

Function Bool
CharIsSymbol(unsigned int c)
{
    return (('!' <= c && c <= '/') ||
            (':' <= c && c <= '@') ||
            ('[' <= c && c <= '`') ||
            ('{' <= c && c <= '~'));
}

Function Bool
CharIsSpace(unsigned int c)
{
    return (' '  == c ||
            '\t' == c ||
            '\v' == c ||
            '\n' == c ||
            '\r' == c ||
            '\f' == c);
}

Function Bool
CharIsNumber(unsigned int c)
{
    return '0' <= c && c <= '9';
}

Function Bool
CharIsUppercase(unsigned int c)
{
    return 'A' <= c && c <= 'Z';
}

Function Bool
CharIsLowercase(unsigned int c)
{
    return 'a' <= c && c <= 'z';
}

Function Bool
CharIsLetter(unsigned int c)
{
    return CharIsUppercase(c) || CharIsLowercase(c);
}

Function Bool
CharIsAlphanumeric(unsigned int c)
{
    return CharIsLetter(c) || CharIsNumber(c);
}

Function Bool
CharIsPrintable(unsigned int c)
{
    return CharIsAlphanumeric(c) || CharIsSpace(c) || CharIsSymbol(c);
}

Function unsigned int
CharLowerFromUpper(unsigned int c)
{
    unsigned int result = c;
    if(CharIsUppercase(c))
    {
        result ^= 1 << 5;
    }
    return result;
}

Function unsigned int
CharUpperFromLower(unsigned int c)
{
    unsigned int result = c;
    if(CharIsLowercase(c))
    {
        result ^= 1 << 5;
    }
    return result;
}

//~NOTE(tbt): c-string helpers

Function size_t
CStringCalculateUTF8Length(char *cstring)
{
    size_t len = 0;
    S8 str = { .buffer = cstring, .len = ~((size_t)0) };
    
    UTFConsume consume = CodepointFromUTF8(str, len);
    while(True)
    {
        if('\0' == consume.codepoint)
        {
            return len;
        }
        else
        {
            len += consume.advance;
            consume = CodepointFromUTF8(str, len);
        }
    }
}

Function size_t
CStringCalculateUTF16Length(wchar_t *cstring)
{
    size_t len = 0;
    S16 str = { .buffer = cstring, .len = ~((size_t)0) };
    
    UTFConsume consume = CodepointFromUTF16(str, len);
    while(True)
    {
        if(0 == consume.codepoint)
        {
            return len;
        }
        else
        {
            len += consume.advance;
            consume = CodepointFromUTF16(str, len);
        }
    }
}

Function S8
CStringAsS8(char *cstring)
{
    S8 result =
    {
        .buffer = cstring,
        .len = CStringCalculateUTF8Length(cstring),
    };
    return result;
}

Function S16
CStringAsS16(wchar_t *cstring)
{
    S16 result =
    {
        .buffer = cstring,
        .len = CStringCalculateUTF16Length(cstring),
    };
    return result;
}

//~NOTE(tbt): unicode conversions

Function Bool
UTF8IsContinuationByte(S8 string, int index)
{
    unsigned char bit_7 = !!(string.buffer[index] & Bit(7));
    unsigned char bit_6 = !!(string.buffer[index] & Bit(6));
    Bool result = (1 == bit_7 && 0 == bit_6);
    return result;
}

Function UTFConsume
CodepointFromUTF8(S8 string, int index)
{
    UTFConsume result = { ~((unsigned int)0), 1 };
    
    if(index < string.len && !UTF8IsContinuationByte(string, index))
    {
        int max = string.len - index;
        
        unsigned char utf8_class[32] = 
        {
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
        };
        
        unsigned int bitmask_3 = 0x07;
        unsigned int bitmask_4 = 0x0F;
        unsigned int bitmask_5 = 0x1F;
        unsigned int bitmask_6 = 0x3F;
        
        unsigned char byte = string.buffer[index];
        unsigned char byte_class = utf8_class[byte >> 3];
        
        switch(byte_class)
        {
            case(1):
            {
                result.codepoint = byte;
            } break;
            
            case(2):
            {
                if(2 <= max)
                {
                    unsigned char cont_byte = string.buffer[index + 1];
                    if(0 == utf8_class[cont_byte >> 3])
                    {
                        result.codepoint = (byte & bitmask_5) << 6;
                        result.codepoint |= (cont_byte & bitmask_6);
                        result.advance = 2;
                    }
                }
            } break;
            
            case(3):
            {
                if (3 <= max)
                {
                    unsigned char cont_byte[2] = { string.buffer[index + 1], string.buffer[index + 2] };
                    if (0 == utf8_class[cont_byte[0] >> 3] &&
                        0 == utf8_class[cont_byte[1] >> 3])
                    {
                        result.codepoint = (byte & bitmask_4) << 12;
                        result.codepoint |= ((cont_byte[0] & bitmask_6) << 6);
                        result.codepoint |= (cont_byte[1] & bitmask_6);
                        result.advance = 3;
                    }
                }
            } break;
            
            case(4):
            {
                if(4 <= max)
                {
                    unsigned char cont_byte[3] =
                    {
                        string.buffer[index + 1],
                        string.buffer[index + 2],
                        string.buffer[index + 3]
                    };
                    if(0 == utf8_class[cont_byte[0] >> 3] &&
                       0 == utf8_class[cont_byte[1] >> 3] &&
                       0 == utf8_class[cont_byte[2] >> 3])
                    {
                        result.codepoint = (byte & bitmask_3) << 18;
                        result.codepoint |= (cont_byte[0] & bitmask_6) << 12;
                        result.codepoint |= (cont_byte[1] & bitmask_6) << 6;
                        result.codepoint |= (cont_byte[2] & bitmask_6);
                        result.advance = 4;
                    }
                }
            } break;
        }
    }
    return result;
}

Function S16
S16FromS8(M_Arena *arena, S8 string)
{
    S16 result = {0};
    
    UTFConsume consume;
    for(size_t i = 0;
        i < string.len;
        i += consume.advance)
    {
        consume = CodepointFromUTF8(string, i);
        // NOTE(tbt): UTF16FromCodepoint allocates unaligned so characters guaranteed to be
        //            contiguous in arena - no need to copy anything :)
        S16 s16 = UTF16FromCodepoint(arena, consume.codepoint);
        if (!result.buffer)
        {
            result.buffer = s16.buffer;
        }
        result.len += s16.len;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

Function S32
S32FromS8(M_Arena *arena, S8 string)
{
    S32 result = {0};
    
    UTFConsume consume;
    for(size_t i = 0;
        i < string.len;
        i += consume.advance)
    {
        consume = CodepointFromUTF8(string, i);
        
        unsigned int *character = M_ArenaPushAligned(arena, sizeof(*character), 1);
        if (!result.buffer)
        {
            result.buffer = character;
        }
        *character = consume.codepoint;
        result.len += 1;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

Function UTFConsume
CodepointFromUTF16(S16 string, int index)
{
    int max = string.len - index;
    
    UTFConsume result = { string.buffer[index + 0], 1 };
    if(1 < max &&
       0xD800 <= string.buffer[index + 0] && string.buffer[index + 0] < 0xDC00 &&
       0xDC00 <= string.buffer[index + 1] && string.buffer[index + 1] < 0xE000)
    {
        result.codepoint = ((string.buffer[index + 0] - 0xD800) << 10) | (string.buffer[index + 1] - 0xDC00);
        result.advance = 2;
    }
    return result;
}

Function S8
S8FromS16(M_Arena *arena, S16 string)
{
    S8 result = {0};
    
    UTFConsume consume;
    for(size_t i = 0;
        i < string.len;
        i += consume.advance)
    {
        consume = CodepointFromUTF16(string, i);
        // NOTE(tbt): UTF8FromCodepoint allocates unaligned so characters guaranteed to be
        //            contiguous in arena - no need to copy anything :)
        S8 s8 = UTF8FromCodepoint(arena, consume.codepoint);
        if (!result.buffer)
        {
            result.buffer = s8.buffer;
        }
        result.len += s8.len;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

Function S32
S32FromS16(M_Arena *arena, S16 string)
{
    S32 result = {0};
    
    UTFConsume consume;
    for(size_t i = 0;
        i < string.len;
        i += consume.advance)
    {
        consume = CodepointFromUTF16(string, i);
        
        unsigned int *character = M_ArenaPushAligned(arena, sizeof(*character), 1);
        if (!result.buffer)
        {
            result.buffer = character;
        }
        *character = consume.codepoint;
        result.len += 1;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

// NOTE(tbt): unlike most other string Functions, does not allocate and write a null
//            terminator after the buffer
Function S8
UTF8FromCodepoint(M_Arena *arena, unsigned int codepoint)
{
    S8 result;
    
    if(codepoint == ~((unsigned int)0))
    {
        result.len = 1;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = '?';
    }
    if(codepoint <= 0x7f)
    {
        result.len = 1;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = codepoint;
    }
    else if(codepoint <= 0x7ff)
    {
        result.len = 2;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = 0xc0 | (codepoint >> 6);
        result.buffer[1] = 0x80 | (codepoint & 0x3f);
    }
    else if(codepoint <= 0xffff)
    {
        result.len = 3;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = 0xe0 | ((codepoint >> 12));
        result.buffer[1] = 0x80 | ((codepoint >>  6) & 0x3f);
        result.buffer[2] = 0x80 | ((codepoint & 0x3f));
    }
    else if(codepoint <= 0xffff)
    {
        result.len = 4;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = 0xf0 | ((codepoint >> 18));
        result.buffer[1] = 0x80 | ((codepoint >> 12) & 0x3f);
        result.buffer[2] = 0x80 | ((codepoint >>  6) & 0x3f);
        result.buffer[3] = 0x80 | ((codepoint & 0x3f));
    }
    
    return result;
}

// NOTE(tbt): unlike most other string Functions, does not allocate and write a null
//            terminator after the buffer
Function S16
UTF16FromCodepoint(M_Arena *arena, unsigned int codepoint)
{
    S16 result;
    
    if(codepoint == ~((unsigned int)0))
    {
        result.len = 1;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = '?';
    }
    else if(codepoint < 0x10000)
    {
        result.len = 1;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        result.buffer[0] = codepoint;
    }
    else
    {
        result.len = 2;
        result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
        size_t v = codepoint - 0x10000;
        result.buffer[0] = '?';
        result.buffer[0] = 0xD800 + (v >> 10);
        result.buffer[1] = 0xDC00 + (v & 0x03FF);
    }
    
    return result;
}

Function S8
S8FromS32(M_Arena *arena, S32 string)
{
    S8 result = {0};
    
    for(size_t i = 0;
        i < string.len;
        i += 1)
    {
        // NOTE(tbt): UTF8FromCodepoint allocates unaligned so characters guaranteed to be
        //            contiguous in arena - no need to copy anything :)
        S8 s8 = UTF8FromCodepoint(arena, string.buffer[i]);
        if (!result.buffer)
        {
            result.buffer = s8.buffer;
        }
        result.len += s8.len;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

Function S16
S16FromS32(M_Arena *arena, S32 string)
{
    S16 result = {0};
    
    for(size_t i = 0;
        i < string.len;
        i += 1)
    {
        // NOTE(tbt): UTF16FromCodepoint allocates unaligned so characters guaranteed to be
        //            contiguous in arena - no need to copy anything :)
        S16 s16 = UTF16FromCodepoint(arena, string.buffer[i]);
        if (!result.buffer)
        {
            result.buffer = s16.buffer;
        }
        result.len += s16.len;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

//~NOTE(tbt): string operations

Function S8
S8Clone(M_Arena *arena, S8 string)
{
    S8 result;
    result.len = string.len;
    result.buffer = M_ArenaPushAligned(arena, string.len + 1, 1);
    M_Copy(result.buffer, string.buffer, result.len);
    return result;
}

Function S8
S8CloneFL(M_FreeList *free_list, S8 string)
{
    S8 result;
    result.len = string.len;
    result.buffer = M_FreeListAlloc(free_list, string.len + 1);
    M_Copy(result.buffer, string.buffer, result.len);
    return result;
}

Function S16
S16Clone(M_Arena *arena, S16 string)
{
    S16 result;
    size_t size = string.len*sizeof(string.buffer[0]);
    result.len = string.len;
    result.buffer = M_ArenaPushAligned(arena, size + sizeof(wchar_t), 1);
    M_Copy(result.buffer, string.buffer, size);
    return result;
}

Function S16
S16CloneFL(M_FreeList *free_list, S16 string)
{
    S16 result;
    size_t size = string.len*sizeof(string.buffer[0]);
    result.len = string.len;
    result.buffer = M_FreeListAlloc(free_list, size + sizeof(wchar_t));
    M_Copy(result.buffer, string.buffer, size);
    return result;
}

Function S8
S8PrefixGet(S8 string, size_t len)
{
    S8 result =
    {
        .len = Min1U(len, string.len),
        .buffer = string.buffer,
    };
    return result;
}

Function S8
S8SuffixGet(S8 string, size_t len)
{
    S8 result =
    {
        .len = Min1U(len, string.len),
        .buffer = string.buffer,
    };
    result.buffer += string.len - result.len;
    return result;
}

Function size_t
S8ByteIndexFromCharIndex(S8 string, size_t char_index)
{
    size_t i = 0;
    for(UTFConsume consume = CodepointFromUTF8(string, i);
        char_index > 0;
        char_index -= 1)
    {
        i += consume.advance;
        consume = CodepointFromUTF8(string, i);
    }
    return i;
}

Function size_t
S8CharIndexFromByteIndex(S8 string, size_t byte_index)
{
    size_t char_index = 0;
    if(byte_index <= string.len)
    {
        size_t i = 0;
        for(UTFConsume consume = CodepointFromUTF8(string, i);
            i < byte_index;
            i += consume.advance)
        {
            char_index += 1;
            consume = CodepointFromUTF8(string, i);
        }
    }
    return char_index;
}

Function S8
S8FromFmtV(M_Arena *arena, char *fmt, va_list args)
{
    S8 result;
    
    va_list _args;
    va_copy(_args, args);
    result.len = stbsp_vsnprintf(0, 0, fmt, args);
    result.buffer = M_ArenaPush(arena, result.len + 1);
    stbsp_vsnprintf(result.buffer, result.len + 1, fmt, _args);
    
    return result;
}

Function S8
S8FromFmt(M_Arena *arena, char *fmt, ...)
{
    S8 result;
    va_list args;
    va_start(args, fmt);
    result = S8FromFmtV(arena, fmt, args);
    va_end(args);
    return result;
}

Function Bool
S8Match(S8 a, S8 b, MatchFlags flags)
{
    Bool is_match;
    
    if(!(flags & MatchFlags_RightSideSloppy) && a.len != b.len)
    {
        is_match = False;
    }
    else
    {
        size_t length_to_compare = Min(a.len, b.len);
        
        // TODO(tbt): work with unicode
        if(flags & MatchFlags_CaseInsensitive)
        {
            is_match = True;
            for(size_t character_index = 0;
                character_index < length_to_compare;
                character_index += 1)
            {
                if(CharLowerFromUpper(a.buffer[character_index]) !=
                   CharLowerFromUpper(b.buffer[character_index]))
                {
                    is_match = False;
                    break;
                }
            }
        }
        else
        {
            is_match = M_Compare(a.buffer, b.buffer, length_to_compare);
        }
    }
    
    return is_match;
}

Function S8
S8Substring(S8 h, S8 n, MatchFlags flags)
{
    S8 result = {0};
    
    // NOTE(tbt): doesn't make any sense
    Assert(0 == (flags & MatchFlags_RightSideSloppy));
    
    for(size_t character_index = 0;
        n.len <= h.len - character_index;
        character_index += 1)
    {
        S8 comp = { &h.buffer[character_index], n.len };
        if(S8Match(comp, n, flags))
        {
            result = comp;
            break;
        }
    }
    
    return result;
}

Function Bool
S8HasSubstring(S8 h, S8 n, MatchFlags flags)
{
    Bool result = (S8Substring(h, n, flags).len > 0);
    return result;
}

Function S8
S8Advance(S8 *string, size_t n)
{
    size_t to_advance = Min1U(n, string->len);
    S8 result =
    {
        .buffer = string->buffer,
        .len = n,
    };
    string->buffer += n;
    string->len -= n;
    return result;
}

Function Bool
S8Consume(S8 *string, S8 consume)
{
    Bool result = False;
    if(S8Match(*string, consume, MatchFlags_RightSideSloppy))
    {
        S8Advance(string, consume.len);
        result = True;
    }
    return result;
}

Function S8
S8Strip(M_Arena *arena, S8 a, int b)
{
    S8 result = S8Clone(arena, a);
    
    UTFConsume consume;
    for(size_t character_index = 0;
        character_index < result.len;
        character_index += consume.advance)
    {
        consume = CodepointFromUTF8(result, character_index);
        
        if(b == consume.codepoint)
        {
            M_Copy(&result.buffer[character_index],
                   &result.buffer[character_index + consume.advance],
                   result.len - (character_index - consume.advance));
            
            character_index -= consume.advance;
            result.len -= consume.advance;
        }
    }
    result.buffer[result.len] = '\0';
    
    return result;
};

Function Bool
S16Match(S16 a, S16 b, MatchFlags flags)
{
    Bool is_match;
    
    if(!(flags & MatchFlags_RightSideSloppy) && a.len != b.len)
    {
        is_match = False;
    }
    else
    {
        size_t length_to_compare = Min(a.len, b.len);
        
        // TODO(tbt): work with unicode
        if(flags & MatchFlags_CaseInsensitive)
        {
            is_match = True;
            for(size_t character_index = 0;
                character_index < length_to_compare;
                character_index += 1)
            {
                if(CharLowerFromUpper(a.buffer[character_index]) !=
                   CharLowerFromUpper(b.buffer[character_index]))
                {
                    is_match = False;
                    break;
                }
            }
        }
        else
        {
            is_match = M_Compare(a.buffer, b.buffer, length_to_compare);
        }
    }
    
    return is_match;
}

#include "external/MurmurHash3.cpp"

Function uint32_t
S8Hash(S8 string)
{
    uint32_t result;
    MurmurHash3_x86_32(string.buffer, string.len*sizeof(string.buffer[0]), 0, &result);
    return result;
}

Function uint32_t
S16Hash(S16 string)
{
    uint32_t result;
    MurmurHash3_x86_32(string.buffer, string.len*sizeof(string.buffer[0]), 0, &result);
    return result;
}

Function S8
S8LFFromCRLF(M_Arena *arena, S8 string)
{
    S8 result = {0};
    
    UTFConsume consume;
    for(size_t i = 0;
        i < string.len;
        i += consume.advance)
    {
        consume = CodepointFromUTF8(string, i);
        UTFConsume next_ch = CodepointFromUTF8(string, i + consume.advance);
        if('\r' == consume.codepoint &&
           '\n' == next_ch.codepoint)
        {
            char *c = M_ArenaPushAligned(arena, 1, 1);
            *c = '\n';
            if(0 == result.buffer)
            {
                result.buffer = c;
            }
            result.len += 1;
            i += next_ch.advance;
        }
        else
        {
            char *c = M_ArenaPushAligned(arena, consume.advance, 1);
            M_Copy(c, &string.buffer[i], consume.advance);
            if(0 == result.buffer)
            {
                result.buffer = c;
            }
            result.len += consume.advance;
        }
    }
    
    return result;
}

Function Bool
FilenameHasTrailingSlash(S8 filename)
{
    Bool result = (filename.len > 0 && DirectorySeparatorChr == filename.buffer[filename.len - 1]);
    return result;
}

Function S8
ExtensionFromFilename(S8 filename)
{
    char *last_dot = 0;
    
    UTFConsume consume = CodepointFromUTF8(filename, 0);
    for(size_t character_index = 0;
        character_index < filename.len;
        character_index += consume.advance)
    {
        if('.' == consume.codepoint)
        {
            last_dot = &filename.buffer[character_index - consume.advance];
        }
        
        consume = CodepointFromUTF8(filename, character_index);
    }
    
    S8 result =
    {
        .buffer = last_dot,
        .len = filename.len - (last_dot - filename.buffer),
    };
    if(0 == last_dot)
    {
        result.buffer = &filename.buffer[filename.len];
        result.len = 0;
    }
    
    return result;
}

Function S8
FilenamePush(M_Arena *arena, S8 filename, S8 string)
{
    S8 result;
    
    if(FilenameHasTrailingSlash(filename) || 0 == filename.len)
    {
        result = S8FromFmt(arena, "%.*s%.*s",
                           FmtS8(filename),
                           FmtS8(string));
    }
    else
        
    {
        result = S8FromFmt(arena, "%.*s" DirectorySeparatorStr "%.*s",
                           FmtS8(filename),
                           FmtS8(string));
    }
    
    return result;
}

Function S8
FilenamePop(S8 filename)
{
    char *last_slash = 0;
    
    UTFConsume consume = CodepointFromUTF8(filename, 0);
    for(size_t character_index = 0;
        character_index < filename.len;
        character_index += consume.advance)
    {
        if(DirectorySeparatorChr == consume.codepoint)
        {
            last_slash = &filename.buffer[character_index];
        }
        
        consume = CodepointFromUTF8(filename, character_index);
    }
    
    S8 result = {0};
    if(0 != last_slash)
    {
        result.buffer = filename.buffer;
        result.len = last_slash - filename.buffer - 1;
    }
    
    return result;
}

Function S8
FilenameLast(S8 filename)
{
    char *last_slash = 0;
    
    UTFConsume consume = CodepointFromUTF8(filename, 0);
    for(size_t character_index = 0;
        character_index < filename.len;
        character_index += consume.advance)
    {
        if(DirectorySeparatorChr == consume.codepoint)
        {
            last_slash = &filename.buffer[character_index];
        }
        
        consume = CodepointFromUTF8(filename, character_index);
    }
    
    S8 result = filename;
    if(0 != last_slash)
    {
        result.buffer = last_slash;
        result.len = &filename.buffer[filename.len] - last_slash;
    }
    
    return result;
}

Function Bool
FilenameIsChild(S8 parent, S8 filename)
{
    Bool result = False;
    
    while(!result && parent.len > 0)
    {
        if(S8Match(parent, filename, 0))
        {
            result  = True;
        }
        parent = FilenamePop(parent);
    }
    
    return result;
}

Function Bool
S8IsWordBoundary(S8 string, size_t index)
{
    Bool result;
    
    if (0 < index && index < string.len)
    {
        result = ((CharIsSpace(string.buffer[index - 1]) || CharIsSymbol(string.buffer[index - 1])) &&
                  (!CharIsSpace(string.buffer[index]) &&
                   !CharIsSymbol(string.buffer[index])));
    }
    else
    {
        result = True;
    }
    
    return result;
}

Function double
S8Parse1F(S8 string)
{
    double result = 0.0f;
    double sign = 1.0f;
    
    if(0 == string.len)
    {
        result = NaN;
    }
    else
    {
        if('-' == string.buffer[0])
        {
            sign = -1.0f;
            S8Advance(&string, 1);
        }
        
        Bool past_radix = False;
        float radix = 1.0f;
        
        while(string.len > 0)
        {
            if(CharIsNumber(string.buffer[0]))
            {
                if(past_radix)
                {
                    radix *= 10.0f;
                }
                result *= 10.0f;
                result += string.buffer[0] - '0';
            }
            else if(!past_radix && '.' == string.buffer[0])
            {
                past_radix = True;
            }
            else
            {
                result = NaN;
                break;
            }
            
            S8Advance(&string, 1);
        }
        
        result = sign*(result / radix);
    }
    
    return result;
}

//~NOTE(tbt): string lists

Function void
S8ListPushExplicit(S8List *list, S8Node *string)
{
    if(0 == list->first)
    {
        Assert(0 == list->last);
        string->next = 0;
        list->first = string;
        list->last = string;
    }
    else
    {
        string->next = list->first;
        list->first = string;
    }
    list->count += 1;
}

Function void
S8ListAppendExplicit(S8List *list, S8Node *string)
{
    if(0 == list->last)
    {
        Assert(0 == list->first);
        list->first = string;
        list->last = string;
    }
    else
    {
        list->last->next = string;
        list->last = string;
    }
    string->next = 0;
    list->count += 1;
}

Function S8Node *
S8ListPush(M_Arena *arena, S8List *list, S8 string)
{
    S8Node *node = M_ArenaPush(arena, sizeof(*node));
    node->string = S8Clone(arena, string);
    S8ListPushExplicit(list, node);
    return node;
}

Function S8Node *
S8ListAppend(M_Arena *arena, S8List *list, S8 string)
{
    S8Node *node = M_ArenaPush(arena, sizeof(*node));
    node->string = S8Clone(arena, string);
    S8ListAppendExplicit(list, node);
    return node;
}

Function void
S8ListConcatenate(M_Arena *arena, S8List *a, S8List b)
{
    for(S8ListForEach(b, s))
    {
        S8ListAppend(arena, a, s->string);
    }
}

Function S8List
S8ListClone(M_Arena *arena, S8List list)
{
    S8List result = {0};
    for(S8ListForEach(list, node))
    {
        S8ListAppend(arena, &result, node->string);
    }
    return result;
}

Function S8
S8ListJoin(M_Arena *arena, S8List list)
{
    S8 result = {0};
    for(S8ListForEach(list, node))
    {
        char *buffer = M_ArenaPushAligned(arena, node->string.len, 1);
        M_Copy(buffer, node->string.buffer, node->string.len);
        if(0 == result.buffer)
        {
            result.buffer = buffer;
        }
        result.len += node->string.len;
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

Function S8
S8ListJoinFormated(M_Arena *arena, S8List list, S8ListJoinFormat join)
{
    S8 result = {0};
    
    size_t prefix_and_suffix_len = join.prefix.len + join.suffix.len;
    
    for(S8ListForEach(list, node))
    {
        char *buffer = M_ArenaPushAligned(arena, node->string.len + prefix_and_suffix_len, 1);
        result.len += node->string.len + prefix_and_suffix_len;
        M_Copy(buffer, join.prefix.buffer, join.prefix.len);
        M_Copy(buffer + join.prefix.len, node->string.buffer, node->string.len);
        M_Copy(buffer + join.prefix.len + node->string.len, join.suffix.buffer, join.suffix.len);
        if(0 != node->next)
        {
            char *delimiter_copy = M_ArenaPushAligned(arena, join.delimiter.len, 1);
            result.len += join.delimiter.len;
            M_Copy(delimiter_copy, join.delimiter.buffer, join.delimiter.len);
        }
        if(0 == result.buffer)
        {
            result.buffer = buffer;
        }
    }
    // NOTE(tbt): null terminate to make easy to use with system APIs
    M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
    
    return result;
}

Function S8
S8ListS8FromIndex(S8List list,
                  size_t index)
{
    S8 result = {0};
    
    S8Node *node = list.first;
    for(size_t i = 0;
        i < index && 0 != node;
        node = node->next)
    {
        i += 1;
    }
    if(0 != node)
    {
        result = node->string;
    }
    return result;
}

Function S8ListFind
S8ListIndexFromS8(S8List list,
                  S8 string,
                  MatchFlags flags)
{
    S8ListFind result = {0};
    for(S8ListForEach(list, i))
    {
        if(S8Match(string, i->string, flags))
        {
            result.has = True;
            break;
        }
        result.i += 1;
    }
    return result;
}

Function Bool
S8ListHasS8(S8List list,
            S8 string,
            MatchFlags flags)
{
    Bool result = False;
    for(S8ListForEach(list, s))
    {
        if(S8Match(string, s->string, flags))
        {
            result = True;
            break;
        }
    }
    return result;
}

Function void
S8ListRecalculate(S8List *list)
{
    // TODO(tbt): this is kind of dumb
    list->count = 0;
    list->last = 0;
    for(S8ListForEach(*list, s))
    {
        list->last = s;
        list->count += 1;
    }
}

Function void
S8ListRemoveExplicit(S8List *list, S8Node *string)
{
    for(S8Node **s = &list->first;
        0 != *s;
        s = &(*s)->next)
    {
        if(*s == string)
        {
            *s = string->next;
            break;
        }
    }
    S8ListRecalculate(list);
}

Function S8Node *
S8ListRemoveFirstOccurenceOf(S8List *list,
                             S8 string,
                             MatchFlags flags)
{
    S8Node *result = 0;
    for(S8Node **s = &list->first;
        0 == result && 0 != (*s);
        s = &(*s)->next)
    {
        if(S8Match(string, (*s)->string, flags))
        {
            result = (*s);
            *s = (*s)->next;
            list->count -= 1;
            result->next = 0;
        }
    }
    S8ListRecalculate(list);
    return result;
}

Function void
S8ListRemoveAllOccurencesOf(S8List *list,
                            S8 string,
                            MatchFlags flags)
{
    for(S8Node **s = &list->first;
        0 != *s;
        s = &(*s)->next)
    {
        if(S8Match(string, (*s)->string, flags))
        {
            *s = (*s)->next;
            list->count -= 1;
            if(0 == (*s))
            {
                break;
            }
        }
    }
    S8ListRecalculate(list);
}

//~NOTE(tbt): compression

Function S8
S8Deflate(M_Arena *arena, S8 string)
{
    S8 result = {0};
    
    if(string.len > 0)
    {
        unsigned char out[Kilobytes(256)];
        
        z_stream stream = {0};
        if(Z_OK == deflateInit(&stream, Z_DEFAULT_COMPRESSION))
        {
            stream.avail_in = string.len;
            stream.next_in = string.buffer;
            
            do {
                stream.avail_out = sizeof(out);
                stream.next_out = out;
                deflate(&stream, Z_FINISH);
                
                size_t have = sizeof(out) - stream.avail_out;
                char *buffer = M_ArenaPushAligned(arena, have, 1);
                if(0 == result.buffer)
                {
                    result.buffer = buffer;
                }
                M_Copy(result.buffer + result.len, out, have);
                result.len += have;
            } while (stream.avail_out == 0);
            
            deflateEnd(&stream);
        }
    }
    
    return result;
}

Function S8
S8Inflate(M_Arena *arena, S8 string)
{
    S8 result = {0};
    
    if(string.len > 0)
    {
        unsigned char out[Kilobytes(256)];
        
        z_stream stream = {0};
        if(Z_OK == inflateInit(&stream))
        {
            int rc;
            do {
                stream.avail_in = string.len;
                stream.next_in = string.buffer;
                
                do {
                    stream.avail_out = sizeof(out);
                    stream.next_out = out;
                    rc = inflate(&stream, Z_NO_FLUSH);
                    switch(rc)
                    {
                        case Z_NEED_DICT:
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                        goto end;
                    }
                    
                    size_t have = sizeof(out) - stream.avail_out;
                    char *buffer = M_ArenaPushAligned(arena, have, 1);
                    if(0 == result.buffer)
                    {
                        result.buffer = buffer;
                    }
                    M_Copy(result.buffer + result.len, out, have);
                    result.len += have;
                } while (stream.avail_out == 0);
            } while (rc != Z_STREAM_END);
            
            end:;
            inflateEnd(&stream);
        }
    }
    
    return result;
}

//~NOTE(tbt): deserialisation

Function uint8_t
S8DeserialiseU8(S8 *data)
{
    uint8_t result = S8ReadType(uint8_t, *data);
    S8Advance(data, sizeof(uint8_t));
    return result;
}

Function uint16_t
S8DeserialiseU16(S8 *data)
{
    uint16_t result = S8ReadType(uint16_t, *data);
    S8Advance(data, sizeof(uint16_t));
    return result;
}

Function uint32_t
S8DeserialiseU32(S8 *data)
{
    uint32_t result = S8ReadType(uint32_t, *data);
    S8Advance(data, sizeof(uint32_t));
    return result;
}

Function uint64_t
S8DeserialiseU64(S8 *data)
{
    uint64_t result = S8ReadType(uint64_t, *data);
    S8Advance(data, sizeof(uint64_t));
    return result;
}

Function int8_t
S8DeserialiseI8(S8 *data)
{
    int8_t result = S8ReadType(int8_t, *data);
    S8Advance(data, sizeof(int8_t));
    return result;
}

Function int16_t
S8DeserialiseI16(S8 *data)
{
    int16_t result = S8ReadType(int16_t, *data);
    S8Advance(data, sizeof(int16_t));
    return result;
}

Function int32_t
S8DeserialiseI32(S8 *data)
{
    int32_t result = S8ReadType(int32_t, *data);
    S8Advance(data, sizeof(int32_t));
    return result;
}

Function int64_t
S8DeserialiseI64(S8 *data)
{
    int64_t result = S8ReadType(int64_t, *data);
    S8Advance(data, sizeof(int64_t));
    return result;
}

Function float
S8Deserialise1F(S8 *data)
{
    float result = S8ReadType(float, *data);
    S8Advance(data, sizeof(float));
    return result;
}

Function V2F
S8Deserialise2F(S8 *data)
{
    V2F result = {0};
    result.x = S8Deserialise1F(data);
    result.y = S8Deserialise1F(data);
    return result;
}

Function V3F
S8Deserialise3F(S8 *data)
{
    V3F result = {0};
    result.x = S8Deserialise1F(data);
    result.y = S8Deserialise1F(data);
    result.z = S8Deserialise1F(data);
    return result;
}

Function V4F
S8Deserialise4F(S8 *data)
{
    V4F result = {0};
    result.x = S8Deserialise1F(data);
    result.y = S8Deserialise1F(data);
    result.z = S8Deserialise1F(data);
    result.w = S8Deserialise1F(data);
    return result;
}

Function void
S8SerialiseBytes(S8 *data, void *src, size_t bytes)
{
    size_t to_copy = Min1U(bytes, data->len);
    M_Copy(data->buffer, src, to_copy);
    S8Advance(data, to_copy);
}