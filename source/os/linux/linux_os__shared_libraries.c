
Function SharedLib
SharedLibMake(S8 filename);
{
    SharedLib result = dlopen(filename.buffer, RTLD_NOW | RTLD_GLOBAL);
    return result;
}

Function void *
SharedLibSymbolGet(SharedLib lib, char *symbol)
{
    void *result = dlsym(lib, symbol);
    return result;
}

Function void
SharedLibDestroy(SharedLib lib)
{
    dlclose(lib);
}
