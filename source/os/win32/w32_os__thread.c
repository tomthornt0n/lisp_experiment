
//~NOTE(tbt): thread creation

Global HANDLE w32_threads_count = 0;

typedef struct
{
    ThreadInitHook *init;
    void *user_data;
    TC_Data tctx;
} W32_ThreadProcArgs;

Function DWORD WINAPI
W32_ThreadProc(LPVOID param)
{
    W32_ThreadProcArgs *args = param;
    
    LONG logical_thread_index;
    ReleaseSemaphore(w32_threads_count, 1, &logical_thread_index);
    TC_Make(&args->tctx, logical_thread_index + 1);
    TC_Set(&args->tctx);
    CoInitializeEx(0, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    
    args->init(args->user_data);
    
    TC_Destroy(&args->tctx);
    LocalFree(param);
    WaitForSingleObject(w32_threads_count, 0);
    
    return 0;
}

Function Thread
ThreadMake(ThreadInitHook *init,
           void *user_data)
{
    W32_ThreadProcArgs *args = LocalAlloc(0, sizeof(*args));
    args->init = init;
    args->user_data = user_data;
    void *result = CreateThread(0, 0, W32_ThreadProc, args, 0, 0);
    return result;
}

Function void
ThreadJoin(Thread thread)
{
    WaitForSingleObject(thread, INFINITE);
}

Function void
ThreadDestroy(Thread thread)
{
    TerminateThread(thread, 0);
}

//~NOTE(tbt): semaphores

Function Semaphore
SemaphoreMake(int initial)
{
    HANDLE sem = CreateSemaphoreW(0, initial, INT_MAX, 0);
    return sem;
}

Function void
SemaphoreSignal(Semaphore sem)
{
    ReleaseSemaphore(sem, 1, 0);
}

Function void
SemaphoreWait(Semaphore sem)
{
    WaitForSingleObject(sem, INFINITE);
}

Function void
SemaphoreDestroy(Semaphore sem)
{
    CloseHandle(sem);
}

//~NOTE(tbt): interlocked operations

Function int
ITL_CompareExchange (volatile int *a, int b, int comperand)
{
    return InterlockedCompareExchange(a, b, comperand);
}

Function int
ITL_Exchange(volatile int *a, int b)
{
    return InterlockedExchange(a, b);
}

Function int
ITL_Increment(volatile int *a)
{
    return InterlockedIncrement(a);
}

Function int
ITL_Decrement(volatile int *a)
{
    return InterlockedDecrement(a);
}

Function void *
ITL_CompareExchangePtr(void *volatile *a, void *b, void *comperand)
{
    return InterlockedCompareExchangePointer(a, b, comperand);
}

Function void *
ITL_ExchangePtr(void *volatile *a, void *b)
{
    return InterlockedExchangePointer(a, b);
}

Function void *
ITL_IncrementPtr(void *volatile *a)
{
    void *result;
    
    // TODO(tbt): tidy this up
    
#if Build_ArchX64
    Assert(8 == sizeof(void *));
    result = (void *)InterlockedIncrement64((LONG64 volatile *)a);
#elif Build_ArchX86
    Assert(4 == sizeof(void *));
    result = (void *)InterlockedIncrement((LONG64 volatile *)a);
#else
#   error TODO(tbt): make ITL_IncrementPtr better
#endif
    
    return result;
}

Function void *
ITL_DecrementPtr(void *volatile *a)
{
    void *result;
    
    // TODO(tbt): tidy this up
    
#if Build_ArchX64
    Assert(8 == sizeof(void *));
    result = (void *)InterlockedDecrement64((LONG64 volatile *)a);
#elif Build_ArchX86
    Assert(4 == sizeof(void *));
    result = (void *)InterlockedDecrement((LONG64 volatile *)a);
#else
#   error TODO(tbt): make ITL_IncrementPtr better
#endif
    
    return result;
}

//~NOTE(tbt): misc utils

Function size_t
ProcessorsCountGet(void)
{
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return system_info.dwNumberOfProcessors;
}

