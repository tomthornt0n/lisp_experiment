
//~NOTE(tbt): base thread context type and Functions

Function void
TC_Make(TC_Data *tctx, int logical_thread_index)
{
    M_ScratchPoolMake(&tctx->scratch_pool, m_default_hooks);
    tctx->permanent_arena = M_ArenaMake(m_default_hooks);
    tctx->logical_thread_index = logical_thread_index;
}

Function void
TC_Destroy(TC_Data *tctx)
{
    M_ScratchPoolDestroy(&tctx->scratch_pool);
    M_ArenaDestroy(&tctx->permanent_arena);
}


//~NOTE(tbt): wrappers and helpers

Function M_Temp
TC_ScratchGet(M_Arena *non_conflict[], int non_conflict_count)
{
    M_Temp result;
    M_ScratchPool *pool = &TC_Get()->scratch_pool;
    result = M_ScratchGet(pool, non_conflict, non_conflict_count);
    return result;
}