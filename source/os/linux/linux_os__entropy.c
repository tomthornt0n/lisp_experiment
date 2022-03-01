
Function void
EntropyGet(void *buffer, size_t size)
{
    getentropy(buffer, size);
}

Function int
IntFromEntropy(void)
{
    int result;
    EntropyGet(&result, sizeof(result));
    return result;
}
