
    Function void *
M_Reserve(size_t size)
{
    return mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

Function void
M_Release(void *memory, size_t size)
{
    int rc = munmap(memory, size);
    Assert_(0 == rc, strerror(errno));
}

Function void
M_Commit(void *memory, size_t size)
{
    int rc = mprotect(memory, size, PROT_READ | PROT_WRITE);
    Assert_(0 == rc, strerror(errno));
}

Function void
M_Decommit(void *memory, size_t size)
{
    int rc = mprotect(memory, size, PROT_NONE);
    Assert_(0 == rc, strerror(errno));
}

