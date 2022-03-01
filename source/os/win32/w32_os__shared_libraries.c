#define SharedLibExport __declspec(dllexport)

Function SharedLib
SharedLibMake(S8 filename)
{
    SharedLib result;
    
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    Assert(sizeof(SharedLib) == sizeof(HMODULE));
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    HMODULE module = LoadLibraryW(filename_s16.buffer);
    result = (void *)module;
    
    M_TempEnd(&scratch);
    return result;
}

Function void *
SharedLibSymbolGet(SharedLib lib, char *symbol)
{
    Assert(0 != lib);
    void *result;
    HMODULE module = (HMODULE)lib;
    FARPROC fn = GetProcAddress(module, symbol);
    result = (void *)fn;
    return result;
}

Function void
SharedLibDestroy(SharedLib lib)
{
    Assert(0 != lib);
    HMODULE module = (HMODULE)lib;
    FreeLibrary(module);
}
