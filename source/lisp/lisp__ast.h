
typedef struct AST_Node AST_Node;
struct AST_Node
{
    AST_Node *next;
    AST_Node *children;
    AST_Node **from;
    size_t children_count;
    S8 value;
    
    RT_Val evaluated;
};

Function AST_Node *AST_TreeFromS8 (M_FreeList *allocator, S8 input);
Function AST_Node *AST_TreeClone   (M_FreeList *allocator, AST_Node *root);
Function void      AST_TreeFree  (M_FreeList *allocator, AST_Node *root);
Function S8        AST_S8FromTree (M_Arena *arena, AST_Node *root, Bool include_siblings);

