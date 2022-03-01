
Function RT_Env *
RT_EnvMake(size_t bucket_count, RT_Env *parent)
{
    M_Arena arena = M_ArenaMake(m_default_hooks);
    RT_Env *env = M_ArenaPush(&arena, sizeof(RT_Env) + sizeof(MAP_S8RT_ValBucket)*bucket_count);
	env->variables.arena = arena;
	env->variables.bucket_count = bucket_count;
    env->parent = parent;
    return env;
}

Function RT_Val
RT_EnvLookup(RT_Env *env, S8 name)
{
    RT_Val result = {0};
    do
    {
        result = RT_VarTableGet(&env->variables, name);
        env = env->parent;
    } while(RT_ValKind_None == result.kind && 0 != env);
    return result;
}

Function RT_Val
RT_EnvVarSet(RT_Env *env, S8 name, RT_Val value)
{
    RT_Env *env_1 = env;
    
    while(0 != env_1 && !RT_VarTableHasKey(&env_1->variables, name))
    {
        env_1 = env_1->parent;
    }
    
    if(0 == env_1)
    {
        env_1 = env;
    }
    
    RT_Val previous_value = RT_VarTableSet(&env_1->variables, name, value);
    return previous_value;
}

Function void
RT_UpdateKeyState(W_Handle window, RT_Env *env)
{
#define KeyDef(NAME, STRING, IS_REAL) if(IS_REAL) { RT_VarTableSet(&env->variables, S8("Key_" #NAME), (RT_Val){ .kind = RT_ValKind_Bool, .bool = 0 == ed_editing && W_KeyStateGet(window, I_ ## Key_ ## NAME) }); }
#include "graphics/graphics__keylist.h"
}

Function V4F
RT_ColFromExpr(AST_Node *expr, RT_Env *env)
{
    V4F col = Col(0.2f, 0.2f, 1.0f, 0.5f);;
    
    Bool is_valid_col = False;
    
    if(0 != expr)
    {
        RT_Val val = RT_ASTExec(expr, env);
        if(RT_ValKind_Colour == val.kind)
        {
            col = Col(val.col.x, val.col.y, val.col.z, val.col.w);
            is_valid_col = True;
        }
    }
    
    if(!is_valid_col)
    {
        RT_Val brush = RT_EnvLookup(env, S8("brush"));
        if(RT_ValKind_Colour == brush.kind)
        {
            col = brush.col;
        }
    }
    
    return col;
}

Function RT_Val
RT_ValFromS8(S8 string, RT_Env *env)
{
    RT_Val result = {0};
    
    float f = S8Parse1F(string);
    if(IsNaN1F(f))
    {
        result = RT_EnvLookup(env, string);
    }
    else
    {
        result.kind = RT_ValKind_Number;
        result.num = f;
    }
    
    return result;
}

Function S8
RT_S8FromVal(M_Arena *arena, RT_Val val)
{
    S8 result = S8("ERROR");
    
    switch(val.kind)
    {
        default: break;
        
        case(RT_ValKind_Number):
        {
            result = S8FromFmt(arena, "%f", val.num);
        } break;
        
        case(RT_ValKind_Bool):
        {
            result = val.bool ? S8("True") : S8("False");
        } break;
        
        case(RT_ValKind_None):
        {
            result = S8("None");
        } break;
        
        case(RT_ValKind_Colour):
        {
            result = S8FromFmt(arena, "rgb(%.2f, %.2f, %.2f)", val.col.x / val.col.w, val.col.y / val.col.w, val.col.z / val.col.w);
        } break;
    }
    
    return result;
}

Function RT_OpKind
RT_OpKindFromS8(S8 string)
{
    RT_OpKind result = 0;
    
    if(S8Match(string, S8("begin"), 0))
    {
        result = RT_OpKind_Begin;
    }
    else if(S8Match(string, S8("begin"), 0))
    {
        result = RT_OpKind_Begin;
    }
    else if(S8Match(string, S8("set!"), 0))
    {
        result = RT_OpKind_Set;
    }
    else if(S8Match(string, S8("set"), 0))
    {
        result = RT_OpKind_SetIfUndefined;
    }
    else if(S8Match(string, S8("+"), 0))
    {
        result = RT_OpKind_Add;
    }
    else if(S8Match(string, S8("-"), 0))
    {
        result = RT_OpKind_Subtract;
    }
    else if(S8Match(string, S8("*"), 0))
    {
        result = RT_OpKind_Multiply;
    }
    else if(S8Match(string, S8("/"), 0))
    {
        result = RT_OpKind_Divide;
    }
    else if(S8Match(string, S8("%"), 0))
    {
        result = RT_OpKind_Modulo;
    }
    else if(S8Match(string, S8("or"), 0))
    {
        result = RT_OpKind_Or;
    }
    else if(S8Match(string, S8("and"), 0))
    {
        result = RT_OpKind_And;
    }
    else if(S8Match(string, S8("<"), 0))
    {
        result = RT_OpKind_LT;
    }
    else if(S8Match(string, S8(">"), 0))
    {
        result = RT_OpKind_GT;
    }
    else if(S8Match(string, S8("<="), 0))
    {
        result = RT_OpKind_LTE;
    }
    else if(S8Match(string, S8(">="), 0))
    {
        result = RT_OpKind_GTE;
    }
    else if(S8Match(string, S8("="), 0))
    {
        result = RT_OpKind_Eq;
    }
    else if(S8Match(string, S8("!="), 0))
    {
        result = RT_OpKind_NEq;
    }
    else if(S8Match(string, S8("lambda"), 0))
    {
        result = RT_OpKind_Lambda;
    }
    else if(S8Match(string, S8("call"), 0))
    {
        result = RT_OpKind_Call;
    }
    else if(S8Match(string, S8("while"), 0))
    {
        result = RT_OpKind_While;
    }
    else if(S8Match(string, S8("if"), 0))
    {
        result = RT_OpKind_If;
    }
    else if(S8Match(string, S8("col"), 0))
    {
        result = RT_OpKind_Col;
    }
    else if(S8Match(string, S8("rect"), 0))
    {
        result = RT_OpKind_Rect;
    }
    else if(S8Match(string, S8("circle"), 0))
    {
        result = RT_OpKind_Circle;
    }
    
    return result;
}

Function RT_Val
RT_ASTExec(AST_Node *root, RT_Env *env)
{
    RT_Val result = {0};
    
    if(0 != root)
    {
        if(root->children_count > 1)
        {
            AST_Node *expr = root->children;
            
            RT_OpKind op = RT_OpKindFromS8(expr->value);
            expr = expr->next;
            
            switch(op)
            {
                default: break;
                
                case(RT_OpKind_Call):
                {
                    RT_Val val = RT_ASTExec(expr, env);
                    expr = expr->next;
                    
                    if(0 != expr && RT_ValKind_Lambda == val.kind)
                    {
                        RT_Env *child_env = RT_EnvMake(256, env);
                        AST_Node *args = expr;
                        for(AST_Node *param = val.proc.params; 0 != param && 0 != args; param = param->next)
                        {
                            RT_VarTableSet(&child_env->variables, param->value, RT_ASTExec(args, env));
                            args = args->next;
                        }
                        RT_ASTExec(val.proc.expr, child_env);
                        RT_VarTableDestroy(&child_env->variables);
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Begin):
                {
                    while(0 != expr)
                    {
                        result = RT_ASTExec(expr, env);
                        expr = expr->next;
                    }
                } break;
                
                case(RT_OpKind_SetIfUndefined):
                {
                    RT_Val prev = RT_EnvLookup(env, expr->value);
                    if(RT_ValKind_None != prev.kind)
                    {
                        break;
                    }
                } // NOTE(tbt): fallthrough
                case(RT_OpKind_Set):
                {
                    if(0 != expr->next)
                    {
                        RT_Val val = RT_ASTExec(expr->next, env);
                        RT_Val prev = RT_EnvVarSet(env, expr->value, val);
                        if(RT_ValKind_Lambda == prev.kind)
                        {
                            AST_TreeFree(&lisp_allocator, prev.proc.expr);
                            AST_TreeFree(&lisp_allocator, prev.proc.params);
                        }
                        result = val;
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Add):
                {
                    result.kind = RT_ValKind_Number;
                    result.num = 0.0f;
                    while(0 != expr)
                    {
                        RT_Val n = RT_ASTExec(expr, env);
                        if(RT_ValKind_Number == n.kind)
                        {
                            result.num += n.num;
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                        expr = expr->next;
                    }
                } break;
                
                case(RT_OpKind_Subtract):
                {
                    result = RT_ASTExec(expr, env);
                    expr = expr->next;
                    
                    if(RT_ValKind_Number == result.kind)
                    {
                        while(0 != expr)
                        {
                            RT_Val n = RT_ASTExec(expr, env);
                            if(RT_ValKind_Number == n.kind)
                            {
                                result.num -= n.num;
                            }
                            else
                            {
                                // TODO(tbt): error reporting
                            }
                            expr = expr->next;
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Multiply):
                {
                    result.kind = RT_ValKind_Number;
                    result.num = 1.0f;
                    while(0 != expr)
                    {
                        RT_Val n = RT_ASTExec(expr, env);
                        if(RT_ValKind_Number == n.kind)
                        {
                            result.num *= n.num;
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                        expr = expr->next;
                    }
                } break;
                
                case(RT_OpKind_Divide):
                {
                    result = RT_ASTExec(expr, env);
                    expr = expr->next;
                    
                    if(RT_ValKind_Number == result.kind)
                    {
                        while(0 != expr)
                        {
                            RT_Val n = RT_ASTExec(expr, env);
                            if(RT_ValKind_Number == n.kind)
                            {
                                result.num /= n.num;
                            }
                            else
                            {
                                // TODO(tbt): error reporting
                            }
                            expr = expr->next;
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Modulo):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            result.kind = RT_ValKind_Number;
                            result.num = Mod1F(l.num, r.num);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Or):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Bool == l.kind &&
                           RT_ValKind_Bool == r.kind)
                        {
                            result.kind = RT_ValKind_Bool;
                            result.bool = l.bool || r.bool;
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_And):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Bool == l.kind &&
                           RT_ValKind_Bool == r.kind)
                        {
                            result.kind = RT_ValKind_Bool;
                            result.bool = l.bool && r.bool;
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_LT):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            result.kind = RT_ValKind_Bool;
                            result.bool = (l.num < r.num);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_GT):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            result.kind = RT_ValKind_Bool;
                            result.bool = (l.num > r.num);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_LTE):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            result.kind = RT_ValKind_Bool;
                            result.bool = (l.num <= r.num);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_GTE):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            result.kind = RT_ValKind_Bool;
                            result.bool = (l.num >= r.num);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Eq):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            float slop = 0.003f;
                            result.kind = RT_ValKind_Bool;
                            result.bool = (Abs1F(l.num - r.num) < slop);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_NEq):
                {
                    if(0 != expr->next)
                    {
                        RT_Val l = RT_ASTExec(expr, env);
                        RT_Val r = RT_ASTExec(expr->next, env);
                        
                        if(RT_ValKind_Number == l.kind &&
                           RT_ValKind_Number == r.kind)
                        {
                            float slop = 0.003f;
                            result.kind = RT_ValKind_Bool;
                            result.bool = !(Abs1F(l.num - r.num) < slop);
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Rect):
                {
                    RT_Val x = RT_ASTExec(expr, env);
                    expr = expr->next;
                    if(0 != expr && RT_ValKind_Number == x.kind)
                    {
                        RT_Val y = RT_ASTExec(expr, env);
                        expr = expr->next;
                        if(0 != expr && RT_ValKind_Number == y.kind)
                        {
                            RT_Val w = RT_ASTExec(expr, env);
                            expr = expr->next;
                            if(0 != expr && RT_ValKind_Number == w.kind)
                            {
                                RT_Val h = RT_ASTExec(expr, env);
                                expr = expr->next;
                                if(RT_ValKind_Number == h.kind)
                                {
                                    I2F rect = RectMake2F(Add2F(V2F(x.num, y.num), lisp_camera), V2F(w.num, h.num));
                                    R_CmdDrawRectFill(rect, RT_ColFromExpr(expr, env));
                                }
                                else
                                {
                                    // TODO(tbt): error reporting
                                }
                            }
                            else
                            {
                                // TODO(tbt): error reporting
                            }
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Circle):
                {
                    RT_Val x = RT_ASTExec(expr, env);
                    expr = expr->next;
                    if(0 != expr && RT_ValKind_Number == x.kind)
                    {
                        RT_Val y = RT_ASTExec(expr, env);
                        expr = expr->next;
                        if(0 != expr && RT_ValKind_Number == y.kind)
                        {
                            RT_Val r = RT_ASTExec(expr, env);
                            expr = expr->next;
                            if(RT_ValKind_Number == r.kind)
                            {
                                R_CmdDrawCircleFill(Add2F(V2F(x.num, y.num), lisp_camera), r.num, RT_ColFromExpr(expr, env));
                            }
                            else
                            {
                                // TODO(tbt): error reporting
                            }
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Lambda):
                {
                    result.kind = RT_ValKind_Lambda;
                    if(0 != expr->next)
                    {
                        result.proc.params = AST_TreeClone(&lisp_allocator, expr->children);
                        result.proc.expr = AST_TreeClone(&lisp_allocator, expr->next);
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_While):
                {
                    AST_Node *cond = expr;
                    expr = expr->next;
                    
                    if(0 != expr)
                    {
                        RT_Val val = RT_ASTExec(cond, env);
                        if(val.kind == RT_ValKind_Bool)
                        {
                            while(val.bool)
                            {
                                result = RT_ASTExec(expr, env);
                                val = RT_ASTExec(cond, env);
                            }
                        }
                        else
                        {
                            // TODO(tbt): error reporting
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_If):
                {
                    AST_Node *cond = expr;
                    expr = expr->next;
                    
                    RT_Val val = RT_ASTExec(cond, env);
                    if(val.kind == RT_ValKind_Bool)
                    {
                        if(val.bool)
                        {
                            result = RT_ASTExec(expr, env);
                        }
                        else if(0 != expr->next)
                        {
                            expr = expr->next;
                            result = RT_ASTExec(expr, env);
                        }
                    }
                    else
                    {
                        // TODO(tbt): error reporting
                    }
                } break;
                
                case(RT_OpKind_Col):
                {
                    result.kind = RT_ValKind_Colour;
                    
                    for(int i = 0; 0 != expr && i < 4; i += 1)
                    {
                        RT_Val val = RT_ASTExec(expr, env);
                        if(RT_ValKind_Number == val.kind)
                        {
                            result.col.elements[i] = Clamp1F(val.num, 0.0f, 1.0f);
                        }
                        else
                        {
                            result.col.elements[i] = 1.0f;
                            // TODO(tbt): error reporting
                        }
                        expr = expr->next;
                    }
                    result.col.x *= result.col.w;
                    result.col.y *= result.col.w;
                    result.col.z *= result.col.w;
                } break;
            }
        }
        else
        {
            if(0 == root->value.len)
            {
                result = RT_ASTExec(root->children, env);
            }
            else
            {
                result = RT_ValFromS8(root->value, env);
            }
        }
        
        root->evaluated = result;
    }
    
    return result;
}
