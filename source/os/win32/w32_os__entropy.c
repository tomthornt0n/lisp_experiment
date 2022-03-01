Function void
EntropyGet(void *buffer, size_t size)
{
    HCRYPTPROV ctx = 0;
    CryptAcquireContextW(&ctx, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(ctx, size, buffer);
    CryptReleaseContext(ctx, 0);
}

Function int
IntFromEntropy(void)
{
    int result;
    EntropyGet(&result, sizeof(result));
    return result;
}
