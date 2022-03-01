
#ifndef HS_ImplementationKey
#error Missing key for hash set implementation
#endif
#ifndef HS_ImplementationHash
#error Missing hash algorithm for hash set implementation
#endif
#ifndef HS_ImplementationMatch
#error Missing key comparison for hash set implementation
#endif
#ifndef HS_ImplementationIllegalKeyMatch
#error Missing illegal key comparison for hash set implementation
#endif
#ifndef HS_ImplementationIllegalKeyVal
#error Missing illegal key value for hash set implementation
#endif
#ifndef HS_ImplementationClone
# define HS_ImplementationClone(A, O) (O)
#endif

#if HS_Bucket
#error Bucket definition already exists when implementing a hash set
#endif
#if HS_Set
#error Set definition already exists when implementing a hash set
#endif

#ifndef HS_BucketDef
# define HS_BucketDef(T) Glue(HS_, Glue(T, Bucket))
#endif
#ifndef HS_SetDef
# define HS_SetDef(T) Glue(Glue(HS_, T), Set)
#endif
#ifndef HS_FnDef
# define HS_FnDef(F) Glue(HS_Set, F)
#endif

#define HS_Bucket HS_BucketDef(HS_ImplementationKey)
#define HS_Set HS_SetDef(HS_ImplementationKey)

#ifndef HS_ForEach
# define HS_ForEach(T, S, V) HS_BucketDef(T) *(V) = (S).items; 0 != (V); (V) = (V)->next
#endif



typedef struct HS_Bucket HS_Bucket;
struct HS_Bucket
{
	HS_Bucket *next_hash;
	HS_Bucket *next;
	HS_ImplementationKey key;
	uint64_t hash;	
};

typedef struct 
{
	M_Arena arena;
	size_t bucket_count;
    HS_Bucket *free;
    HS_Bucket *items;
    size_t count;
    HS_Bucket buckets[];
} HS_Set;

Function HS_Bucket *
HS_FnDef(BucketAllocate)(HS_Set *set)
{
    HS_Bucket *result = set->free;
    if(0 == result)
    {
        result = M_ArenaPush(&set->arena, sizeof(*result));
    }
    else
    {
        set->free = result->next;
    }
    return result;
}

Function HS_Set *
HS_FnDef(Make)(size_t bucket_count)
{
    M_Arena arena = M_ArenaMake(m_default_hooks);
    HS_Set *set = M_ArenaPush(&arena, sizeof(HS_Set) + sizeof(HS_Bucket)*bucket_count);
	set->arena = arena;
	set->bucket_count = bucket_count;
    return set;
}

Function Bool
HS_FnDef(HasKey)(HS_Set *set, HS_ImplementationKey key)
{
    Bool result = False;
    if(!HS_ImplementationIllegalKeyMatch(key))
    {
        uint64_t hash = HS_ImplementationHash(key);
        size_t index = hash % set->bucket_count;
        for(HS_Bucket *bucket = &set->buckets[index];
            False == result && 0 != bucket;
            bucket = bucket->next_hash)
        {
            if(bucket->hash == hash && HS_ImplementationMatch(bucket->key, key))
			{
                result = True;
            }
        }
    }
	return result;
}

Function Bool
HS_FnDef(Insert)(HS_Set *set, HS_ImplementationKey key)
{
    Bool result = False;
	if(!HS_ImplementationIllegalKeyMatch(key))
    {
        if(!HS_FnDef(HasKey)(set, key))
        {
            uint64_t hash = HS_ImplementationHash(key);
            size_t index = hash % set->bucket_count;
            
            // NOTE(tbt): return True if not already in hash set
            result = True;
            
            HS_Bucket *bucket = &set->buckets[index];
            if(!HS_ImplementationIllegalKeyMatch(bucket->key))
            {
                HS_Bucket *new_item = HS_FnDef(BucketAllocate)(set);
                new_item->next_hash = bucket->next_hash;
                bucket->next_hash = new_item;
                bucket = new_item;
            }
            bucket->next = set->items;
            set->items = bucket;
            set->count += 1;
            bucket->key = HS_ImplementationClone(&set->arena, key);
            bucket->hash = hash;
        }
    }
	return result;
}

Function Bool
HS_FnDef(Remove)(HS_Set* set, HS_ImplementationKey key)
{
    Bool result = False;
	if(!HS_ImplementationIllegalKeyMatch(key))
    {
        uint64_t hash = HS_ImplementationHash(key);
        size_t index = hash % set->bucket_count;
        
        HS_Bucket *to_free = 0;
        
        HS_Bucket *bucket = &set->buckets[index];
        if(bucket->hash == hash && HS_ImplementationMatch(bucket->key, key))
        {
            result = True;
            to_free = bucket->next_hash;
            if(0 == to_free)
            {
                for(HS_Bucket **b = &set->items;
                    0 != *b;
                    b = &(*b)->next)
                {
                    if(*b == bucket)
                    {
                        *b = (*b)->next;
                        break;
                    }
                }
                bucket->hash = 0;
                bucket->key = HS_ImplementationIllegalKeyVal;
                set->count -= 1;
            }
            else
            {
                bucket->next_hash = to_free->next_hash;
                bucket->hash = to_free->hash;
                bucket->key = to_free->key;
            }
        }
        else
        {
            for(HS_Bucket **b = &bucket->next_hash;
                0 != (*b);
                b = &(*b)->next_hash)
            {
                if((*b)->hash == hash && HS_ImplementationMatch((*b)->key, key))
                {
                    to_free = *b;
                    (*b) = (*b)->next_hash;
                    result = True;
                    break;
                }
            }
        }
        if(0 != to_free)
        {
            for(HS_Bucket **b = &set->items;
                0 != *b;
                b = &(*b)->next)
            {
                if(*b == to_free)
                {
                    *b = (*b)->next;
                    break;
                }
            }
            to_free->hash = 0;
            to_free->key = HS_ImplementationIllegalKeyVal;
            to_free->next = set->free;
            set->free = to_free;
            set->count -= 1;
        }
    }
    return result;
}

Function void
HS_FnDef(Destroy)(HS_Set *set)
{
    M_ArenaDestroy(&set->arena);
}

#undef HS_Bucket
#undef HS_Set
#undef HS_ImplementationKey
#undef HS_ImplementationHash
#undef HS_ImplementationMatch
#undef HS_ImplementationIllegalKeyMatch
#undef HS_ImplementationIllegalKeyVal
#undef HS_ImplementationClone
