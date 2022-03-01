
typedef struct AST_Node AST_Node;

typedef enum
{
    RT_ValKind_None,
    RT_ValKind_Number,
    RT_ValKind_Bool,
    RT_ValKind_Lambda,
    RT_ValKind_Colour,
} RT_ValKind;

typedef struct
{
    RT_ValKind kind;
    union
    {
        double num;
        struct
        {
            AST_Node *expr;
            AST_Node *params;
        } proc;
        uint32_t bool;
        V4F col;
    };
} RT_Val;

typedef enum
{
    RT_OpKind_NONE,
    RT_OpKind_Begin,
    RT_OpKind_Set,
    RT_OpKind_SetIfUndefined,
    RT_OpKind_Add,
    RT_OpKind_Subtract,
    RT_OpKind_Multiply,
    RT_OpKind_Divide,
    RT_OpKind_Modulo,
    RT_OpKind_Or,
    RT_OpKind_And,
    RT_OpKind_LT,
    RT_OpKind_GT,
    RT_OpKind_LTE,
    RT_OpKind_GTE,
    RT_OpKind_Eq,
    RT_OpKind_NEq,
    RT_OpKind_Call,
    RT_OpKind_Lambda,
    RT_OpKind_While,
    RT_OpKind_If,
    
    RT_OpKind_Col,
    RT_OpKind_Rect,
    RT_OpKind_Circle,
} RT_OpKind;

#define MAP_ImplementationName RT_VarTable
#define MAP_ImplementationKey S8
#define MAP_ImplementationVal RT_Val
#define MAP_ImplementationHash(K) (Hash_((K).buffer, (K).len * sizeof((K).buffer[0])))
#define MAP_ImplementationMatch(A, B) (S8Match((A), (B), MatchFlags_Exact))
#define MAP_ImplementationIllegalKeyMatch(K) ((K).len == 0)
#define MAP_ImplementationIllegalKeyVal (S8(""))
#define MAP_ImplementationClone(A, O) S8Clone(A, O)
#include "collections/collections__map.c"
typedef struct RT_Env RT_Env;
struct RT_Env
{
    RT_Env *parent;
    RT_VarTable variables;
};

Function RT_Env *RT_EnvMake   (size_t bucket_count, RT_Env *parent);
Function RT_Val  RT_EnvLookup (RT_Env *env, S8 name);

Global RT_Env *rt_global_env;

Function V4F RT_ColFromExpr (AST_Node *expr, RT_Env *env);

Function RT_Val    RT_ValFromS8    (S8 string, RT_Env *env);
Function RT_OpKind RT_OpKindFromS8 (S8 string);
Function RT_Val    RT_ASTExec      (AST_Node *root, RT_Env *env);
