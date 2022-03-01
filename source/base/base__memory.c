
//~NOTE(tbt): utilities

Function void *
M_Set(void *memory, unsigned char value, size_t bytes)
{
    unsigned char *data = memory;
    for(int i = 0;
        i < bytes;
        i += 1)
    {
        data[i] = value;
    }
    return memory;
}

Function void *
M_Copy(void *restrict dest, const void *restrict src, size_t bytes)
{
    const unsigned char *from = src;
    unsigned char *to = dest;
    for(int i = 0;
        i < bytes;
        i += 1)
    {
        to[i] = from[i];
    }
    return dest;
}

Function Bool
M_Compare(const void *a, const void *b, size_t bytes)
{
    const unsigned char *one = a;
    const unsigned char *two = b;
    for(int i = 0;
        i < bytes;
        i += 1)
    {
        if(one[i] != two[i])
        {
            return False;
        }
    }
    return True;
}

Function uintptr_t
M_AlignForward(uintptr_t ptr, size_t align)
{
    Assert(0 == (align & (align - 1)) && "alignment must be power of two");
    
    uintptr_t result = (uintptr_t)ptr;
    uintptr_t alignment = (uintptr_t)align;
    uintptr_t modulo = result & (alignment - 1);
    if(0 != modulo)
    {
        result += alignment - modulo;
    }
    
    return result;
}

Function size_t
M_PadWithHeader(uintptr_t ptr, size_t align, size_t header_size)
{
    Assert(0 == (align & (align - 1)) && "alignment must be power of two");
    
	uintptr_t modulo = ptr & (align - 1);
    
	uintptr_t padding = 0;
	if(0 != modulo)
    {
		padding = align - modulo;
	}
    
    uintptr_t needed_space = header_size;
	if(padding < needed_space)
    {
		needed_space -= padding;
        
		if(0 != (needed_space & (align-1)))
        {
			padding += align*(1 + (needed_space / align));
		}
        else
        {
			padding += align*(needed_space / align);
		}
	}
    
	return padding;
}

//~NOTE(tbt): arenas

Function M_Arena
M_ArenaMake(M_Hooks hooks)
{
    M_Arena arena = {0};
    arena.hooks = hooks;
    arena.max = m_arena_default_cap;
    arena.base = arena.hooks.reserve_func(arena.max);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    return arena;
}

Function M_Arena
M_ArenaMakeSized(M_Hooks hooks, size_t size)
{
    M_Arena arena = {0};
    arena.hooks = hooks;
    arena.max = size;
    arena.base = arena.hooks.reserve_func(arena.max);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    return arena;
}

Function M_Arena
M_ArenaMakeLocal(void *backing, size_t size)
{
    M_Arena arena = {0};
    arena.max = size;
    arena.alloc_position = 0;
    arena.base = backing;
    arena.commit_position = size;
    return arena;
}

Function M_Arena
M_ArenaMakeFixed(M_Hooks hooks, size_t size)
{
    M_Arena arena = {0};
    arena.hooks = hooks;
    arena.max = size;
    arena.alloc_position = 0;
    arena.base = arena.hooks.reserve_func(arena.max);
    arena.hooks.commit_func(arena.base, arena.max);
    arena.commit_position = arena.max;
    return arena;
}

Function void *
M_ArenaPushAligned(M_Arena *arena, size_t size, size_t align)
{
    void *memory = 0;
    
    uintptr_t new_alloc_position = M_AlignForward(arena->alloc_position + size, align);
    if(arena->alloc_position + size < arena->max)
    {
        if(new_alloc_position > arena->commit_position)
        {
            size_t commit_size = size;
            commit_size += m_arena_commit_chunk_size - 1;
            commit_size -= commit_size % m_arena_commit_chunk_size;
            arena->hooks.commit_func((unsigned char *)arena->base + arena->commit_position, commit_size);
            arena->commit_position += commit_size;
        }
        
        memory = (unsigned char *)arena->base + arena->alloc_position;
        arena->alloc_position = new_alloc_position;
    }
    
    if(0 == memory)
    {
        AssertBreak_();
    }
    
    M_Set(memory, 0, size);
    return memory;
}

Function void *
M_ArenaPush(M_Arena *arena, size_t size)
{
    return M_ArenaPushAligned(arena, size, M_DefaultAlignment);
}

Function void
M_ArenaPop(M_Arena *arena, size_t size)
{
    size_t to_pop = Min1U(size, arena->alloc_position);
    arena->alloc_position -= to_pop;
}

Function void
M_ArenaPopTo(M_Arena *arena, size_t alloc_position)
{
    size_t to_pop = Min1U(alloc_position, arena->alloc_position);
    arena->alloc_position = alloc_position;
}

Function void
M_ArenaClear(M_Arena *arena)
{
    M_ArenaPop(arena, arena->alloc_position);
}

Function void
M_ArenaDestroy(M_Arena *arena)
{
    M_Arena a = *arena;
    a.hooks.decommit_func(a.base, a.commit_position);
    a.hooks.release_func(a.base, a.max);
}

//~NOTE(tbt): temporary memory

Function M_Temp
M_TempBegin(M_Arena *arena)
{
    M_Temp result =
    {
        .arena = arena,
        .checkpoint_alloc_position = arena->alloc_position,
    };
    return result;
}

Function void
M_TempEnd(M_Temp *temp)
{
    M_ArenaPopTo(temp->arena, temp->checkpoint_alloc_position);
}


//~NOTE(tbt): scratch pool

Function void
M_ScratchPoolMake(M_ScratchPool *pool, M_Hooks hooks)
{
    for(int arena_index = 0;
        arena_index < ArrayCount(pool->arenas);
        arena_index += 1)
    {
        M_Arena *arena = &pool->arenas[arena_index];
        *arena = M_ArenaMake(hooks);
    }
}

Function M_Temp
M_ScratchGet(M_ScratchPool *pool,
             M_Arena *non_conflict_array[],
             int non_conflict_count)
{
    M_Temp result = {0};
    
    M_Arena *scratch;
    for(size_t arena_index = 0;
        arena_index < ArrayCount(pool->arenas);
        arena_index += 1)
    {
        scratch = &pool->arenas[arena_index];
        Bool is_conflicting = False;
        for(size_t conflict_index = 0;
            conflict_index < non_conflict_count;
            conflict_index += 1)
        {
            M_Arena *non_conflict = non_conflict_array[conflict_index];
            if(non_conflict == scratch)
            {
                is_conflicting = True;
                break;
            }
        }
        if(!is_conflicting)
        {
            result = M_TempBegin(scratch);
            break;
        }
    }
    
    Assert(0 != result.arena);
    return result;
}

Function void
M_ScratchPoolDestroy(M_ScratchPool *pool)
{
    for(int arena_index = 0;
        arena_index < ArrayCount(pool->arenas);
        arena_index += 1)
    {
        M_Arena *arena = &pool->arenas[arena_index];
        M_ArenaDestroy(arena);
    }
}

//~NOTE(tbt): free list allocator

M_FreeListNode *
M_FreeListFindFirst(M_FreeList *free_list, size_t size, size_t align, size_t *padding, M_FreeListNode **prev)
{
    M_FreeListNode *node = free_list->free_list;
    M_FreeListNode *prev_node = 0;
    
    size_t pad = 0;
    while(0 != node)
    {
        pad = M_PadWithHeader((uintptr_t)node, (uintptr_t)align, sizeof(M_FreeListAllocHeader));
        size_t required_space = size + pad;
        if(node->size >= required_space)
        {
            break;
        }
        prev_node = node;
        node = node->next;
    }
    
    if(0 != pad)
    {
        *padding = pad;
    }
    if(0 != prev)
    {
        *prev = prev_node;
    }
    
    return node;
}

M_FreeListNode *
M_FreeListFindBest(M_FreeList *free_list, size_t size, size_t align, size_t *padding, M_FreeListNode **prev)
{
    // TODO(tbt): there is a bug in here somewhere
    
    M_FreeListNode *result = 0;
    size_t smallest_diff = ~((size_t)0);
    
    size_t pad = 0;
    
    M_FreeListNode *prev_node = 0;
    for(M_FreeListNode *node = free_list->free_list; 0 != node; node = node->next)
    {
        pad = M_PadWithHeader((uintptr_t)node, (uintptr_t)align, sizeof(M_FreeListAllocHeader));
        size_t required_space = size + pad;
        if(node->size >= required_space && (node->size - required_space) < smallest_diff)
        {
            result = node;
            smallest_diff = node->size - required_space;
        }
        prev_node = node;
    }
    
    if(0 != pad)
    {
        *padding = pad;
    }
    if(0 != prev)
    {
        *prev = prev_node;
    }
    
    return result;
}

Function void
M_FreeListNodeInsert(M_FreeListNode **head, M_FreeListNode *prev_node, M_FreeListNode *new_node) {
    if(0 == prev_node)
    {
        new_node->next = *head;
        *head = new_node;
    }
    else
    {
        new_node->next = prev_node->next;
        prev_node->next = new_node;
    }
}

Function void
M_FreeListNodeRemove(M_FreeListNode **head, M_FreeListNode *prev_node, M_FreeListNode *to_delete)
{
    if(0 == prev_node)
    { 
        *head = to_delete->next; 
    }
    else
    { 
        prev_node->next = to_delete->next; 
    } 
}

Function M_FreeList
M_FreeListFromArena(M_Arena arena, M_FreeListPlacementPolicy placement_policy)
{
    M_FreeList result = {0};
    result.arena = arena;
    result.placement_policy = placement_policy;
    M_FreeListClear(&result);
    return result;
}

Function M_FreeList
M_FreeListMake(M_Hooks hooks, M_FreeListPlacementPolicy placement_policy)
{
    M_Arena arena = M_ArenaMake(hooks);
    M_FreeList result = M_FreeListFromArena(arena, placement_policy);
    return result;
}

Function void *
M_FreeListAllocAligned(M_FreeList *free_list, size_t size, size_t align)
{
    size = Max1U(size, sizeof(M_FreeListNode));
    align = Max1U(align, 8);
    
    size_t padding = 0;
    M_FreeListNode *prev_node = 0;
    M_FreeListNode *node = 0;
    if(M_FreeListPlacementPolicy_First == free_list->placement_policy)
    {
        node = M_FreeListFindFirst(free_list, size, align, &padding, &prev_node);
    }
    else if(M_FreeListPlacementPolicy_Best == free_list->placement_policy)
    {
        node = M_FreeListFindBest(free_list, size, align, &padding, &prev_node);
    }
    
    if(0 == node)
    {
        padding = M_PadWithHeader((uintptr_t)node, (uintptr_t)align, sizeof(M_FreeListAllocHeader));
        node = M_ArenaPush(&free_list->arena, size + padding);
        node->size = size + padding;
    }
    
    size_t alignment_padding = padding - sizeof(M_FreeListAllocHeader);
    size_t required_space = size + padding;
    size_t remaining = node->size - required_space;
    
    if(remaining > 0)
    {
        M_FreeListNode *new_node = PtrFromInt(IntFromPtr(node) + required_space);
        new_node->size = remaining;
        M_FreeListNodeInsert(&free_list->free_list, node, new_node);
    }
    
    M_FreeListNodeRemove(&free_list->free_list, prev_node, node);
    
    M_FreeListAllocHeader *header_ptr = PtrFromInt(IntFromPtr(node) + alignment_padding);
    header_ptr->size = required_space;
    header_ptr->padding = alignment_padding;
    
    void *result = PtrFromInt(IntFromPtr(header_ptr) + sizeof(M_FreeListAllocHeader));
    M_Set(result, 0, size);
    
    return result;
}

Function void *
M_FreeListAlloc(M_FreeList *free_list, size_t size)
{
    void *result = M_FreeListAllocAligned(free_list, size, M_DefaultAlignment);
    return result;
}

Function void
M_FreeListFree(M_FreeList *free_list, void *ptr)
{
    if(0 != ptr)
    {
        M_FreeListAllocHeader *header = PtrFromInt(IntFromPtr(ptr) - sizeof(M_FreeListAllocHeader));
        M_FreeListNode *free_node = PtrFromInt(IntFromPtr(header) - header->padding);
        free_node->size = header->size + header->padding;
        free_node->next = 0;
        
        if(0 == free_list->free_list)
        {
            free_list->free_list = free_node;
        }
        else
        {
            M_FreeListNode *prev_node = 0;
            for(M_FreeListNode *node = free_list->free_list; 0 != node; node = node->next)
            {
                if(ptr < (void *)node)
                {
                    break;
                }
                prev_node = node;
            }
            M_FreeListNodeInsert(&free_list->free_list, prev_node, free_node);
            M_FreeListCoalescence(free_list, prev_node, free_node);
        }
    }
}

Function void
M_FreeListCoalescence(M_FreeList *free_list, M_FreeListNode *prev_node, M_FreeListNode *node)
{
    if(0 != prev_node)
    {
        if(0 != node->next && PtrFromInt(IntFromPtr(node) + node->size) == node->next)
        {
            node->size += node->next->size;
            M_FreeListNodeRemove(&free_list->free_list, node, node->next);
        }
        
        if(0 != prev_node->next && PtrFromInt(IntFromPtr(prev_node) + prev_node->size) == node)
        {
            prev_node->size += prev_node->next->size;
            M_FreeListNodeRemove(&free_list->free_list, prev_node, node);
        }
    }
}

Function void
M_FreeListClear(M_FreeList *free_list)
{
    M_ArenaClear(&free_list->arena);
    free_list->free_list = 0;
}

Function void
M_FreeListDestroy(M_FreeList *free_list)
{
    M_ArenaDestroy(&free_list->arena);
}
