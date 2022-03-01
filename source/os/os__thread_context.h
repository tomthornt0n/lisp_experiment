
//~NOTE(tbt): base thread context type and Functions

typedef struct
{
    int logical_thread_index;
    
    M_ScratchPool scratch_pool;
    M_Arena permanent_arena;
    
    S8 exe_path;
} TC_Data;

Function void  TC_Make    (TC_Data *tctx, int logical_thread_index);
Function void  TC_Destroy (TC_Data *tctx);

Function TC_Data *TC_Get  (void);         // NOTE(tbt): gets the thread local context pointer
Function void     TC_Set  (TC_Data *ptr); // NOTE(tbt): sets the thread local context pointer

//~NOTE(tbt): wrappers and helpers

Function M_Temp TC_ScratchGet (M_Arena *non_conflict[], int non_conflict_count);
