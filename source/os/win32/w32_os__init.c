Function void
W32_AssertLogCallback(char *title, char *msg)
{
    ConsoleOutputFmt("%s\n", msg);
    MessageBoxA(0, msg, title, MB_ICONERROR | MB_OK);
}

Function void
OS_Init(void)
{
    // NOTE(tbt): weird COM stuff
    CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    OleInitialize(0);
    
    // NOTE(tbt): initialise time stuff
    LARGE_INTEGER perf_counter_frequency;
    if(QueryPerformanceFrequency(&perf_counter_frequency))
    {
        w32_t_perf_counter_ticks_per_second = perf_counter_frequency.QuadPart;
    }
    if(TIMERR_NOERROR == timeBeginPeriod(1))
    {
        w32_t_is_granular = True;
    }
    
    // NOTE(tbt): initialise thread context
    w32_tc_id = TlsAlloc();
    Persist TC_Data main_thread_context;
    TC_Make(&main_thread_context, 0);
    TC_Set(&main_thread_context);
    w32_threads_count = CreateSemaphoreW(0, 0, INT_MAX, 0);
    
    // NOTE(tbt): initialise console
#if Build_ModeDebug
    W32_ConsoleInit();
#endif
    assert_log = W32_AssertLogCallback;
    
    // NOTE(tbt): initialise RNG
    RandIntInit(IntFromEntropy());
}

Function void
OS_Cleanup(void)
{
    if(w32_t_is_granular)
    {
        timeEndPeriod(1);
    }
}
