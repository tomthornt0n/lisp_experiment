
#ifndef MAP_ImplementationKey
#error Missing key for hash map implementation
#endif
#ifndef MAP_ImplementationVal
#error Missing value for hash map implementation
#endif
#ifndef MAP_ImplementationHash
#error Missing hash algorithm for hash map implementation
#endif
#ifndef MAP_ImplementationMatch
#error Missing key comparison for hash map implementation
#endif
#ifndef MAP_ImplementationIllegalKeyMatch
#error Missing illegal key comparison for hash map implementation
#endif
#ifndef MAP_ImplementationIllegalKeyVal
#error Missing illegal key value for hash map implementation
#endif
#ifndef MAP_ImplementationClone
# define MAP_ImplementationClone(A, O) (O)
#endif

#ifdef MAP_Bucket
#error Bucket definition already exists when implementing a hash map
#endif
#ifdef MAP_Map
#error Map definition already exists when implementing a hash map
#endif

#ifndef MAP_BucketDef
# define MAP_BucketDef(K, V) Glue(MAP_, Glue(Glue(K, V), Bucket))
#endif
#ifndef MAP_MapDef
# define MAP_MapDef(K, V) Glue(MAP_, Glue(K, V))
#endif

#define MAP_Bucket MAP_BucketDef(MAP_ImplementationKey, MAP_ImplementationVal)

#ifndef MAP_ImplementationName
# define MAP_Map MAP_MapDef(MAP_ImplementationKey, MAP_ImplementationVal)
#else
# define MAP_Map MAP_ImplementationName
#endif

#ifndef MAP_FnDef
# define MAP_FnDef(F) Glue(MAP_Map, F)
#endif

#ifndef MAP_ForEach
# define MAP_ForEach(T, S, V) MAP_BucketDef(T) *(V) = (S).items; 0 != (V); (V) = (V)->next
#endif

typedef struct MAP_Bucket MAP_Bucket;
struct MAP_Bucket
{
	MAP_Bucket *next_hash;
	MAP_Bucket *next;
	uint64_t hash;	
	MAP_ImplementationKey key;
	MAP_ImplementationVal val;
};

typedef struct MAP_Map MAP_Map;
struct MAP_Map
{
	M_Arena arena;
	size_t bucket_count;
    MAP_Bucket *free;
    MAP_Bucket *items;
    size_t count;
    MAP_Bucket buckets[];
};

Function MAP_Bucket *
MAP_FnDef(BucketAllocate)(MAP_Map *map)
{
    MAP_Bucket *result = map->free;
    if(0 == result)
    {
        result = M_ArenaPush(&map->arena, sizeof(*result));
    }
    else
    {
        map->free = result->next;
    }
    return result;
}

Function MAP_Map *
MAP_FnDef(Make)(size_t bucket_count)
{
    M_Arena arena = M_ArenaMake(m_default_hooks);
    MAP_Map *map = M_ArenaPush(&arena, sizeof(MAP_Map) + sizeof(MAP_Bucket)*bucket_count);
	map->arena = arena;
	map->bucket_count = bucket_count;
    return map;
}

// NOTE(tbt): sets the value corresponding to `key` and returns the previous value
//            inserts a new key if it is not already in the map
Function MAP_ImplementationVal
MAP_FnDef(Set)(MAP_Map *map, MAP_ImplementationKey key, MAP_ImplementationVal val)
{
    MAP_ImplementationVal prev_value = {0};
    
	if(!MAP_ImplementationIllegalKeyMatch(key))
    {
        uint64_t hash = MAP_ImplementationHash(key);
        size_t index = hash % map->bucket_count;
        
        Bool has_key = False;
        if(!MAP_ImplementationIllegalKeyMatch(key))
        {
            for(MAP_Bucket *bucket = &map->buckets[index];
                0 != bucket;
                bucket = bucket->next_hash)
            {
                if(bucket->hash == hash && MAP_ImplementationMatch(bucket->key, key))
                {
                    has_key = True;
                    bucket->val = val;
                    break;
                }
            }
        }
        
        if(!has_key)
        {
            MAP_Bucket *bucket = &map->buckets[index];
            if(!MAP_ImplementationIllegalKeyMatch(bucket->key))
            {
                MAP_Bucket *new_item = MAP_FnDef(BucketAllocate)(map);
                new_item->next_hash = bucket->next_hash;
                bucket->next_hash = new_item;
                bucket = new_item;
            }
            bucket->next = map->items;
            map->items = bucket;
            map->count += 1;
            bucket->hash = hash;
            bucket->key = MAP_ImplementationClone(&map->arena, key);
            bucket->val = val;
        }
    }
	return prev_value;
}

Function MAP_ImplementationVal
MAP_FnDef(Get)(MAP_Map *map, MAP_ImplementationKey key)
{
    MAP_ImplementationVal result = {0};
    if(!MAP_ImplementationIllegalKeyMatch(key))
    {
        uint64_t hash = MAP_ImplementationHash(key);
        size_t index = hash % map->bucket_count;
        for(MAP_Bucket *bucket = &map->buckets[index];
            0 != bucket;
            bucket = bucket->next_hash)
        {
            if(bucket->hash == hash && MAP_ImplementationMatch(bucket->key, key))
			{
                result = bucket->val;
                break;
            }
        }
    }
	return result;
}

Function Bool
MAP_FnDef(HasKey)(MAP_Map *map, MAP_ImplementationKey key)
{
    Bool result = False;
    if(!MAP_ImplementationIllegalKeyMatch(key))
    {
        uint64_t hash = MAP_ImplementationHash(key);
        size_t index = hash % map->bucket_count;
        for(MAP_Bucket *bucket = &map->buckets[index];
            False == result && 0 != bucket;
            bucket = bucket->next_hash)
        {
            if(bucket->hash == hash && MAP_ImplementationMatch(bucket->key, key))
			{
                result = True;
            }
        }
    }
	return result;
}

Function MAP_ImplementationVal
MAP_FnDef(Pop)(MAP_Map* map, MAP_ImplementationKey key)
{
    MAP_ImplementationVal result = {0};
    
	if(!MAP_ImplementationIllegalKeyMatch(key))
    {
        uint64_t hash = MAP_ImplementationHash(key);
        size_t index = hash % map->bucket_count;
        
        MAP_Bucket *to_free = 0;
        
        MAP_Bucket *bucket = &map->buckets[index];
        if(bucket->hash == hash && MAP_ImplementationMatch(bucket->key, key))
        {
            result = bucket->val;
            to_free = bucket->next_hash;
            if(0 == to_free)
            {
                for(MAP_Bucket **b = &map->items;
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
                bucket->key = MAP_ImplementationIllegalKeyVal;
                map->count -= 1;
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
            for(MAP_Bucket **b = &bucket->next_hash;
                0 != (*b);
                b = &(*b)->next_hash)
            {
                if((*b)->hash == hash && MAP_ImplementationMatch((*b)->key, key))
                {
                    to_free = *b;
                    (*b) = (*b)->next_hash;
                    result = to_free->val;
                    break;
                }
            }
        }
        if(0 != to_free)
        {
            for(MAP_Bucket **b = &map->items;
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
            to_free->key = MAP_ImplementationIllegalKeyVal;
            to_free->next = map->free;
            map->free = to_free;
            map->count -= 1;
        }
    }
    return result;
}

Function void
MAP_FnDef(Destroy)(MAP_Map *map)
{
    M_ArenaDestroy(&map->arena);
}

#undef MAP_Bucket
#undef MAP_Map
#undef MAP_ImplementationKey
#undef MAP_ImplementationVal
#undef MAP_ImplementationHash
#undef MAP_ImplementationMatch
#undef MAP_ImplementationIllegalKeyMatch
#undef MAP_ImplementationIllegalKeyVal
#undef MAP_ImplementationClone
#undef MAP_ImplementationName
