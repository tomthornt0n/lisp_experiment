
Global DWORD w32_tc_id = 0;

Function TC_Data *
TC_Get(void)
{
    TC_Data *result = TlsGetValue(w32_tc_id);
    return result;
}

Function void
TC_Set(TC_Data *tctx)
{
    void *ptr = tctx;
    TlsSetValue(w32_tc_id, ptr);
}