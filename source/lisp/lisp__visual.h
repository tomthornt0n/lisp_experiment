
typedef struct
{
    I2F bounds;
    Bool is_populated;
} VIS_LayoutCacheEntry;
#define MAP_ImplementationName VIS_LayoutCache
#define MAP_ImplementationKey S8
#define MAP_ImplementationVal VIS_LayoutCacheEntry
#define MAP_ImplementationHash(K) (Hash_((K).buffer, (K).len * sizeof((K).buffer[0])))
#define MAP_ImplementationMatch(A, B) (S8Match((A), (B), MatchFlags_Exact))
#define MAP_ImplementationIllegalKeyMatch(K) ((K).len == 0)
#define MAP_ImplementationIllegalKeyVal (S8(""))
#define MAP_ImplementationClone(A, O) S8Clone(A, O)
#include "collections/collections__map.c"

typedef struct VIS_LayoutCache VIS_LayoutCache;
Function VIS_LayoutCache     *VIS_LayoutCacheMake    (size_t bucket_count);
Function VIS_LayoutCacheEntry VIS_LayoutCacheSet     (VIS_LayoutCache *map, S8 key, VIS_LayoutCacheEntry val);
Function VIS_LayoutCacheEntry VIS_LayoutCacheGet     (VIS_LayoutCache *map, S8 key);
Function Bool                 VIS_LayoutCacheHasKey  (VIS_LayoutCache *map, S8 key);
Function VIS_LayoutCacheEntry VIS_LayoutCachePop     (VIS_LayoutCache *map, S8 key);
Function void                 VIS_LayoutCacheDestroy (VIS_LayoutCache *map);

Global VIS_LayoutCache *vis_layout_cache;

Function I2F VIS_BoundsFromS8 (S8 string, V2F position);

Function S8   VIS_S8FromBinOp (RT_OpKind op);
Function void VIS_ASTRender   (W_Handle window, AST_Node *root, V2F pos);
