
//~NOTE(tbt): thread creation

typedef void *Thread;

typedef void ThreadInitHook(void *user_data);

#if Build_ModeDebug
# define AssertMainThread() Statement( Assert_(TC_Get()->logical_thread_index == 0, __FUNCTION__ " should only be called from the main thread\n"); )
#else
# define AssertMainThread() Statement( 0; )
#endif

Function Thread ThreadMake    (ThreadInitHook *init, void *user_data);
Function void   ThreadJoin    (Thread thread);
Function void   ThreadDestroy (Thread thread);

//~NOTE(tbt): semaphores

typedef void *Semaphore;

Function Semaphore SemaphoreMake    (int initial);
Function void      SemaphoreSignal  (Semaphore sem);
Function void      SemaphoreWait    (Semaphore sem);
Function void      SemaphoreDestroy (Semaphore sem);

//~NOTE(tbt): interlocked operations

#if Build_CompilerMSVC
# define ITL_Barrier Statement( _WriteBarrier(); _mm_sfence(); )
#elif Build_CompilerGCC || Build_CompilerClang
# define ITL_Barrier Statement( __sync_synchronize(); )
#else
# error ITL_Barrier not implemented for this platform
#endif

Function int ITL_CompareExchange      (volatile int *a, int ex, int comperand);
Function int ITL_Exchange             (volatile int *a, int ex);
Function int ITL_Increment            (volatile int *a);
Function int ITL_Decrement            (volatile int *a);

Function void *ITL_CompareExchangePtr (void *volatile *a, void *ex, void *comperand);
Function void *ITL_ExchangePtr        (void *volatile *a, void *ex);
Function void *ITL_IncrementPtr       (void *volatile *a);
Function void *ITL_DecrementPtr       (void *volatile *a);

//~NOTE(tbt): misc utils

Function size_t ProcessorsCountGet (void); // NOTE(tbt): returns the available number of CPU cores
