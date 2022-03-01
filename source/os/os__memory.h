
//~NOTE(tbt): base memory

Function void *M_Reserve  (size_t size);
Function void  M_Release  (void *memory, size_t size);
Function void  M_Commit   (void *memory, size_t size);
Function void  M_Decommit (void *memory, size_t size);

Global M_Hooks m_default_hooks =
{
    .reserve_func  = M_Reserve,
    .release_func  = M_Release,
    .commit_func   = M_Commit,
    .decommit_func = M_Decommit,
};
