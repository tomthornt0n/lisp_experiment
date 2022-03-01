Function void
CLIP_TextSet(S8 string)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    S16 s16 = S16FromS8(scratch.arena, string);
    OpenClipboard(0);
    EmptyClipboard();
    HGLOBAL mem_handle = GlobalAlloc(GMEM_ZEROINIT, (s16.len + 1)*sizeof(wchar_t));
    void *mem = GlobalLock(mem_handle);
    M_Copy(mem, s16.buffer, s16.len*sizeof(wchar_t));
    GlobalUnlock(mem_handle);
    SetClipboardData(CF_UNICODETEXT, mem_handle);
    CloseClipboard();
    
    M_TempEnd(&scratch);
}

Function void
CLIP_FilesSet(S8List filenames)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    OpenClipboard(0);
    EmptyClipboard();
    S16 file_list = W32_FileOpListFromS8List(scratch.arena, filenames);
    HGLOBAL drop_files_handle = GlobalAlloc(GHND, sizeof(DROPFILES) + (file_list.len + 2)*sizeof(wchar_t));
    DROPFILES *drop_files = GlobalLock(drop_files_handle);
    drop_files->pFiles = sizeof(DROPFILES);
    drop_files->fWide = True;
    void *files_ptr = PtrFromInt(IntFromPtr(drop_files) + drop_files->pFiles);
    M_Copy(files_ptr, file_list.buffer, file_list.len*sizeof(wchar_t));
    GlobalUnlock(drop_files_handle);
    SetClipboardData(CF_HDROP, drop_files_handle);
    CloseClipboard();
    
    M_TempEnd(&scratch);
}

Function S8
CLIP_TextGet(M_Arena *arena)
{
    S8 result = {0};
    
    OpenClipboard(0);
    HGLOBAL mem_handle = GetClipboardData(CF_UNICODETEXT);
    if(0 != mem_handle)
    {
        void *mem = GlobalLock(mem_handle);
        if(0 != mem)
        {
            result = S8FromS16(arena, CStringAsS16(mem));
        }
        GlobalUnlock(mem_handle);
    }
    CloseClipboard();
    
    return result;
}

Function S8List
CLIP_FilesGet(M_Arena *arena)
{
    S8List result = {0};
    
    OpenClipboard(0);
    HGLOBAL mem_handle = GetClipboardData(CF_HDROP);
    if(0 != mem_handle)
    {
        DROPFILES *mem = GlobalLock(mem_handle);
        if(0 != mem)
        {
            wchar_t *string_list = PtrFromInt(IntFromPtr(mem) + mem->pFiles);
            result = W32_S8ListFromWin32StringList(arena, string_list);
        }
        GlobalUnlock(mem_handle);
    }
    CloseClipboard();
    
    return result;
}