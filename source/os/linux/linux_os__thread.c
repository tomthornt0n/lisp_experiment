
//~NOTE(tbt): thread creation

Global int linux_thread_count = 0;

typedef struct
{
    ThreadInitHook *init;
    void *user_data;
    TC_Data tctx;
} LINUX_ThreadProcArgs;

Function void *
LINUX_ThreadProc(void *param)
{
    LINUX_ThreadProcArgs *args = param;
    
    // TODO(tbt): 
    
    int logical_thread_index = __sync_fetch_and_add(&linux_thread_count, 1);
    TC_Make(&args->tctx, logical_thread_index + 1);
    TC_Set(&args->tctx);
    
    args->init(args->user_data);
    
    TC_Destroy(&args->tctx);
    free(param);
    __sync_sub_and_fetch(&linux_thread_count, 1);
    
    return 0;
}

Function Thread
ThreadMake(ThreadInitHook *init,
           void *user_data)
{
    LINUX_ThreadProcArgs *args = malloc(sizeof(*args));
    args->init = init;
    args->user_data = user_data;
    pthread_t result;
    pthread_create(&result, 0, LINUX_ThreadProc, args);
    return PtrFromInt(result);
}

Function void
ThreadJoin(Thread thread)
{
    pthread_t thread_id = IntFromPtr(thread);
    pthread_join(thread_id, 0);
}

Function void
ThreadDestroy(Thread thread)
{
    // TODO(tbt): implement
}

//~NOTE(tbt): semaphores

Function Semaphore
SemaphoreMake(int initial)
{
    sem_t *sem = malloc(sizeof(*sem));
    sem_init(sem, False, initial);
    return sem;
}

Function void
SemaphoreSignal(Semaphore sem)
{
    sem_post(sem);
}

Function void
SemaphoreWait(Semaphore sem)
{
    sem_wait(sem);
}

Function void
SemaphoreDestroy(Semaphore sem)
{
    sem_destroy(sem);
    free(sem);
}

//~NOTE(tbt): interlocked operations

Function int
ITL_CompareExchange(volatile int *a, int b, int comperand)
{
    return __sync_val_compare_and_swap(a, comperand, b);
}

Function int
ITL_Exchange(volatile int *a, int b)
{
    __sync_synchronize();
    return __sync_lock_test_and_set(a, b);
}

Function int
ITL_Increment(volatile int *a)
{
    return __sync_add_and_fetch(a, 1);
}

Function int
ITL_Decrement(volatile int *a)
{
    return __sync_sub_and_fetch(a, 1);
}

Function void *
ITL_CompareExchangePtr(void *volatile *a, void *b, void *comperand)
{
    return __sync_val_compare_and_swap(a, comperand, b);
}

Function void *
ITL_ExchangePtr(void *volatile *a, void *b)
{
    __sync_synchronize();
    return __sync_lock_test_and_set(a, b);
}

Function void *
ITL_IncrementPtr(void *volatile *a)
{
    return __sync_add_and_fetch(a, 1);
}

Function void *
ITL_DecrementPtr(void *volatile *a)
{
    return __sync_sub_and_fetch(a, 1);
}

//~NOTE(tbt): misc.

// TODO(tbt): ProcessorsCountGet()

Function size_t
ProcessorsCountGet(void)
{
    size_t result = sysconf(_SC_NPROCESSORS_ONLN);
    return result;
}
