
Function int
WrapToBounds(int a, float min, float max)
{
    int result = (a < 0) ? -a : a;
    int range = max - min;
    result = min + ((result + range) % range);
    return result;
}


Function void *M_Copy(void *restrict dest, const void *restrict src, size_t bytes);

Function void
Swap(unsigned char *a,
     unsigned char *b,
     int64_t bytes_per_item)
{
    unsigned char temp[1024];
    int64_t w = sizeof(temp);
    if(bytes_per_item < w)
    {
        w = bytes_per_item;
    }
    
    for(int64_t i = 0;
        i < bytes_per_item;
        i += w)
    {
        M_Copy(temp, a + i, w);
        M_Copy(a + i, b + i, w);
        M_Copy(b + i, temp, w);
    }
}

Function int
MinInI(int elements[], size_t n)
{
    int result = INT_MAX;
    for(size_t i = 0; i < n; i += 1)
    {
        result = Min1I(result, elements[i]);
    }
    return result;
}

Function size_t
MinInU(size_t elements[], size_t n)
{
    size_t result = ~((size_t)0);
    for(size_t i = 0; i < n; i += 1)
    {
        result = Min1U(result, elements[i]);
    }
    return result;
}

Function float
MinInF(float elements[], size_t n)
{
    float result = FLT_MAX;
    for(size_t i = 0; i < n; i += 1)
    {
        result = Min1F(result, elements[i]);
    }
    return result;
}

Function int
MaxInI(int elements[], size_t n)
{
    int result = INT_MIN;
    for(size_t i = 0; i < n; i += 1)
    {
        result = Max1I(result, elements[i]);
    }
    return result;
}

Function size_t
MaxInU(size_t elements[], size_t n)
{
    size_t result = 0;
    for(size_t i = 0; i < n; i += 1)
    {
        result = Max1U(result, elements[i]);
    }
    return result;
}

Function float
MaxInF(float elements[], size_t n)
{
    float result = FLT_MIN;
    for(size_t i = 0; i < n; i += 1)
    {
        result = Max1F(result, elements[i]);
    }
    return result;
}

Function void
Assert_(int c, char *msg)
{
    if(!c)
    {
        if(0 != assert_log)
        {
            assert_log("Assertion Failure", msg);
        }
        
        AssertBreak_();
    }
}

Function uint64_t
I64EncodeAsU64(int64_t a)
{
    uint64_t result = 0;
    result |= (uint64_t)a <<  1;
    result |= (uint64_t)a >> 63;
    return result;
}

Function int64_t
I64DecodeFromU64(uint64_t a)
{
    int64_t result = a >> 1;
    if(a & 1)
    {
        result *= -1;
    }
    return result;
}
