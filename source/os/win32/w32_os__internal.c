
Function uint64_t
W32_U64FromHiAndLoWords(DWORD hi,
                        DWORD lo)
{
    uint64_t result = 0;
    result |= ((uint64_t)hi) << 32;
    result |= ((uint64_t)lo) <<  0;
    return result;
}

Function S16
W32_FileOpListFromS8List(M_Arena *arena, S8List list)
{
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    
    S8List _list = {0};
    for(S8ListForEach(list, s))
    {
        S8 _s = AbsolutePathFromRelativePath(scratch.arena, s->string);
        _s.len += 1;
        _s = S8Clone(scratch.arena, _s);
        _s.buffer[_s.len - 1] = '\0';
        S8ListAppend(scratch.arena, &_list, _s);
    }
    S8ListAppend(scratch.arena, &_list, S8("\0"));
    
    S8 joined_list = S8ListJoin(scratch.arena, _list);
    S16 result = S16FromS8(arena, joined_list);
    
    M_TempEnd(&scratch);
    return result;
}

Function S8List
W32_S8ListFromWin32StringList(M_Arena *arena,
                              wchar_t *string_list)
{
    S8List result = {0};
    for(;;)
    {
        S16 str = CStringAsS16(string_list);
        string_list += str.len + 1;
        if(0 == str.len)
        {
            break;
        }
        else
        {
            S8Node *node = M_ArenaPush(arena, sizeof(*node));
            node->string = S8FromS16(arena, str);
            S8ListAppendExplicit(&result, node);
        }
    }
    return result;
}

Function S16
W32_FormatLastError(S16 function)
{
    DWORD dw = GetLastError(); 
    
    void *buffer;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &buffer,
                  0, NULL );
    
    S16 result = CStringAsS16(buffer);
    return result;
}