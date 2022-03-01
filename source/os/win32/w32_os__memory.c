
Function void *
M_Reserve(size_t size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

Function void
M_Release(void *memory, size_t size)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

Function void
M_Commit(void *memory, size_t size)
{
    VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

Function void
M_Decommit(void *memory, size_t size)
{
    VirtualFree(memory, size, MEM_DECOMMIT);
}
