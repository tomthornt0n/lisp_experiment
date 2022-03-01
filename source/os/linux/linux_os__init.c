
Function void
LINUX_AssertLogCallback(char *title, char *msg)
{
    ConsoleOutputFmt("\e[4;31m%s\e[0m\n\t%s\n", title, msg);
}

Function void
OS_Init(void)
{
    m_arena_commit_chunk_size = getpagesize();
    
    Persist TC_Data main_thread_context;
    TC_Make(&main_thread_context, 0);
    TC_Set(&main_thread_context);
    
    assert_log = LINUX_AssertLogCallback;
    
    RandIntInit(IntFromEntropy());
}

Function void
OS_Cleanup(void)
{
    return;
}
