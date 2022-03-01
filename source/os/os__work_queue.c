

Function void
WQ_Make(WQ_Queue *queue)
{
    M_Set(queue, 0, sizeof(*queue));
    queue->semaphore = SemaphoreMake(0);
}

// NOTE(tbt): only call from ONE thread
Function void
WQ_Push(WQ_Queue *queue, WQ_Work *work, void *user_data)
{
    uint32_t new_next_write = (queue->next_write + 1) % WQ_EntryCount;
    Assert(new_next_write != queue->next_read);
    WQ_Entry *entry = &queue->entries[queue->next_write];
    entry->work = work;
    entry->user_data = user_data;
    queue->completion_goal += 1;
    ITL_Barrier;
    queue->next_write = new_next_write;
    SemaphoreSignal(queue->semaphore);
}

Function Bool
WQ_DoNextEntry(WQ_Queue *queue)
{
    Bool should_sleep = True;
    
    uint32_t next_read = queue->next_read;
    uint32_t new_next_read = (next_read + 1) % WQ_EntryCount;
    if(next_read != queue->next_write)
    {
        uint32_t index = ITL_CompareExchange(&queue->next_read, new_next_read, next_read);
        if(next_read == index)
        {
            WQ_Entry entry = queue->entries[index];
            entry.work(entry.user_data);
            ITL_Increment(&queue->completion_count);
        }
        should_sleep = False;
    }
    
    return should_sleep;
}

Function void
WQ_CompleteAll(WQ_Queue *queue)
{
    while(queue->completion_count != queue->completion_goal)
    {
        WQ_DoNextEntry(queue);
    }
    queue->completion_goal = 0;
    queue->completion_count = 0;
}

Function void
WQ_ThreadProc(void *user_data)
{
    WQ_Queue *queue = user_data;
    for(;;)
    {
        if(WQ_DoNextEntry(queue))
        {
            SemaphoreWait(queue->semaphore);
        }
    }
}

Function WQ_Queue *
WQ_MakeAndSpawnThreadPool(M_Arena *arena, size_t threads_count)
{
    WQ_Queue *result = M_ArenaPush(arena, sizeof(*result));
    WQ_Make(result);
    
    for(size_t thread_index = 0;
        thread_index < threads_count;
        thread_index += 1)
    {
        ThreadMake(WQ_ThreadProc, result);
    }
    
    return result;
}
