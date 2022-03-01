
Function uint64_t
Hash_(void *buffer, size_t size)
{
    uint64_t hash[2];
    MurmurHash3_x86_128(buffer, size, 0, &hash);
    return hash[0];
}

//~NOTE(tbt): S8 hash set

#define HS_ImplementationKey S8
#define HS_ImplementationHash(K) (Hash_((K).buffer, (K).len * sizeof((K).buffer[0])))
#define HS_ImplementationMatch(A, B) (S8Match((A), (B), MatchFlags_Exact))
#define HS_ImplementationIllegalKeyMatch(K) ((K).len == 0)
#define HS_ImplementationIllegalKeyVal (S8(""))
#define HS_ImplementationClone(A, O) S8Clone(A, O)
#include "collections__hash_set.c"

//~NOTE(tbt): S16 hash set

#define HS_ImplementationKey S16
#define HS_ImplementationHash(K) (Hash_((K).buffer, (K).len * sizeof((K).buffer[0])))
#define HS_ImplementationMatch(A, B) (S16Match((A), (B), MatchFlags_Exact))
#define HS_ImplementationIllegalKeyMatch(K) ((K).len == 0)
#define HS_ImplementationIllegalKeyVal (S16(""))
#define HS_ImplementationClone(A, O) S16Clone(A, O)
#include "collections__hash_set.c"
