
enum { WQ_EntryCount = 4096, };

typedef void WQ_Work(void *user_data);

typedef struct
{
    WQ_Work *work;
    void *user_data;
} WQ_Entry;

typedef struct
{
    Semaphore semaphore;
    volatile uint32_t next_read;
    volatile uint32_t next_write;
    volatile uint32_t completion_goal;
    volatile uint32_t completion_count;
    WQ_Entry entries[WQ_EntryCount];
} WQ_Queue;

Function void WQ_Make        (WQ_Queue *queue);
Function void WQ_Push        (WQ_Queue *queue, WQ_Work *work, void *user_data); // NOTE(tbt): only call from ONE thread
Function Bool WQ_DoNextEntry (WQ_Queue *queue);
Function void WQ_CompleteAll (WQ_Queue *queue);

Function void      WQ_ThreadProc             (void *user_data); // NOTE(tbt): default thread procedure for work queue thread pool
Function WQ_Queue *WQ_MakeAndSpawnThreadPool (M_Arena *arena, size_t threads_count);
