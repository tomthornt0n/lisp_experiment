
Function I2F
VIS_BoundsFromS8(S8 string, V2F position)
{
    I2F result = I2F(position, position);
    
    if(string.len > 0)
    {
        VIS_LayoutCacheEntry entry = VIS_LayoutCacheGet(vis_layout_cache, string);
        
        if(!entry.is_populated)
        {
            M_Temp scratch = TC_ScratchGet(0, 0);
            R_MeasuredText mt = R_MeasureS8(scratch.arena, lisp_font, string, U2F(0.0f));
            entry.is_populated = True;
            entry.bounds = mt.bounds;
            VIS_LayoutCacheSet(vis_layout_cache, string, entry);
            M_TempEnd(&scratch);
        }
        
        result = I2F(Add2F(entry.bounds.min, position), Add2F(entry.bounds.max, position));
    }
    
    return result;
}

Function S8
VIS_S8FromBinOp(RT_OpKind op)
{
    switch(op)
    {
        case(RT_OpKind_LT): return S8(" < ");
        case(RT_OpKind_GT): return S8(" > ");
        case(RT_OpKind_LTE): return S8(" ≤ ");
        case(RT_OpKind_GTE): return S8(" ≥ ");
        case(RT_OpKind_Eq): return S8(" = ");
        case(RT_OpKind_NEq): return S8(" ≠ ");
        case(RT_OpKind_Modulo): return S8(" mod ");
        case(RT_OpKind_Or): return S8(" or ");
        case(RT_OpKind_And): return S8(" and ");
        default: return S8("ERROR");
    }
}

Function S8
VIS_S8FromFn(RT_OpKind op)
{
    switch(op)
    {
        case(RT_OpKind_Rect): return S8("rectangle ");
        case(RT_OpKind_Circle): return S8("circle ");
        case(RT_OpKind_Col): return S8("col ");
        default: return S8("ERROR");
    }
}

Global S8 vis_tooltip = {0};

Function I2F
VIS_ASTRender_(W_Handle window, AST_Node *ast, V2F cur, Bool measure_only)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    I2F bounds = {0};
    
    M_Arena *frame_arena = W_FrameArenaGet(window);
    EV_Queue *events = W_EventQueueGet(window);
    V2F mouse_pos = W_MousePositionGet(window);
    
    Persist R_CmdQueue dummy = {0};
    
    if(measure_only)
    {
        R_CmdQueueRecordingBegin(&dummy);
    }
    
    if(ed_editing == ast)
    {
        R_MeasuredText mt = R_MeasureS8(frame_arena, lisp_editor_font, CStringAsS8(ed_backing), cur);
        UI_DrawS8WithCaret(lisp_editor_font, mt, Col(0.2f, 0.4f, 0.6f, 1.0f), &ed_selection);
        bounds = mt.bounds;
    }
    else
    {
        if(ast->children_count > 1)
        {
            AST_Node *expr = ast->children;
            
            AST_Node *op = expr;
            expr = expr->next;
            
            RT_OpKind op_kind = RT_OpKindFromS8(op->value);
            
            
            switch(op_kind)
            {
                default:
                {
                    I2F text_bounds = VIS_BoundsFromS8(op->value, cur);
                    R_CmdDrawS8(lisp_editor_font, op->value, cur, Col(1.0f, 0.2f, 0.2f, 1.0f));
                    R_CmdDrawRectFill(RectMake2F(V2F(text_bounds.min.x, cur.y), V2F(text_bounds.max.x - text_bounds.min.x, 2.0f)), Col(1.0f, 0.2f, 0.2f, 1.0f));
                    bounds = text_bounds;
                } break;
                
                case(RT_OpKind_Begin):
                {
                    bounds = I2F(U2F(Infinity), U2F(NegativeInfinity));
                    
                    Bool first = True;
                    while(0 != expr)
                    {
                        if(0 != expr && !first)
                        {
                            I2F b = VIS_ASTRender_(window, expr, U2F(0.0f), True);
                            cur.y -= b.min.y;
                        }
                        
                        first = False;
                        
                        I2F child_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.y = child_bounds.max.y + 16.0f;
                        expr = expr->next;
                        
                        bounds.min.x = Min1F(bounds.min.x, child_bounds.min.x);
                        bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                        bounds.max.x = Max1F(bounds.max.x, child_bounds.max.x);
                        bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                    }
                    bounds = Expand2F(bounds, U2F(4.0f));
                    R_CmdDrawRectStroke(bounds, 2.0f, Col(0.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                case(RT_OpKind_Set):
                case(RT_OpKind_SetIfUndefined):
                {
                    S8 string;
                    if(RT_OpKind_SetIfUndefined == op_kind)
                    {
                        string = S8FromFmt(frame_arena, "%.*s := ", FmtS8(expr->value));
                    }
                    else
                    {
                        string = S8FromFmt(frame_arena, "%.*s ← ", FmtS8(expr->value));
                    }
                    R_MeasuredText mt = R_MeasureS8(frame_arena, lisp_font, string, cur);
                    R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0f));
                    cur.x += mt.bounds.max.x - mt.bounds.min.x;
                    expr = expr->next;
                    I2F child_bounds = VIS_ASTRender_(window, expr, cur, False);
                    expr = expr->next;
                    
                    bounds.min.x = mt.bounds.min.x;
                    bounds.min.y = Min1F(mt.bounds.min.y, child_bounds.min.y);
                    bounds.max.x = child_bounds.max.x;
                    bounds.max.y = Max1F(mt.bounds.max.y, child_bounds.max.y);
                } break;
                
                case(RT_OpKind_Add):
                {
                    if(0 != expr->next && 0 == expr->next->next)
                    {
                        I2F left_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.x += left_bounds.max.x - left_bounds.min.x;
                        expr = expr->next;
                        
                        S8 string = S8(" + ");
                        I2F text_bounds = VIS_BoundsFromS8(string, cur);
                        R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0f));
                        cur.x += text_bounds.max.x - text_bounds.min.x;
                        
                        I2F right_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.x += right_bounds.max.x - right_bounds.min.x;
                        expr = expr->next;
                        
                        bounds.min.x = left_bounds.min.x;
                        bounds.min.y = Min1F(left_bounds.min.y, right_bounds.min.y);
                        bounds.max.x = right_bounds.max.x;
                        bounds.max.y = Max1F(left_bounds.max.y, right_bounds.max.y);
                    }
                    else
                    {
                        S8 sigma = S8("∑ {");
                        I2F text_bounds = VIS_BoundsFromS8(sigma, cur);
                        R_CmdDrawS8(lisp_font, sigma, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                        cur.x += text_bounds.max.x - text_bounds.min.x;
                        
                        bounds = text_bounds;
                        
                        while(0 != expr)
                        {
                            I2F child_bounds = VIS_ASTRender_(window, expr, cur, False);
                            cur.x += child_bounds.max.x - child_bounds.min.x;
                            expr = expr->next;
                            
                            if(0 != expr)
                            {
                                S8 comma = S8(", ");
                                I2F text_bounds = VIS_BoundsFromS8(comma, cur);
                                R_CmdDrawS8(lisp_font, comma, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                                cur.x += text_bounds.max.x - text_bounds.min.x;
                                bounds.min.x = Min1F(bounds.min.x, text_bounds.min.x);
                            }
                            else
                            {
                                S8 bracket = S8("}");
                                I2F text_bounds = VIS_BoundsFromS8(bracket, cur);
                                R_CmdDrawS8(lisp_font, bracket, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                                cur.x += text_bounds.max.x - text_bounds.min.x;
                                bounds.max.x = Max1F(bounds.max.x, text_bounds.max.x);
                            }
                            bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                            bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                        }
                    }
                } break;
                
                case(RT_OpKind_Subtract):
                {
                    bounds = I2F(V2F(Infinity, Infinity), V2F(NegativeInfinity, NegativeInfinity));
                    while(0 != expr)
                    {
                        I2F child_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.x += child_bounds.max.x - child_bounds.min.x;
                        expr = expr->next;
                        
                        if(0 != expr)
                        {
                            S8 sub = S8(" − ");
                            I2F text_bounds = VIS_BoundsFromS8(sub, cur);
                            R_CmdDrawS8(lisp_font, sub, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                            cur.x += text_bounds.max.x - text_bounds.min.x;
                        }
                        bounds.min.x = Min1F(bounds.min.x, child_bounds.min.x);
                        bounds.max.x = Max1F(bounds.max.x, child_bounds.max.x);
                        bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                        bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                    }
                } break;
                
                case(RT_OpKind_Multiply):
                {
                    if(0 != expr->next && 0 == expr->next->next)
                    {
                        I2F left_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.x += left_bounds.max.x - left_bounds.min.x;
                        expr = expr->next;
                        
                        S8 string = S8(" × ");
                        I2F text_bounds = VIS_BoundsFromS8(string, cur);
                        R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0f));
                        cur.x += text_bounds.max.x - text_bounds.min.x;
                        
                        I2F right_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.x += right_bounds.max.x - right_bounds.min.x;
                        expr = expr->next;
                        
                        bounds.min.x = left_bounds.min.x;
                        bounds.min.y = Min1F(left_bounds.min.y, right_bounds.min.y);
                        bounds.max.x = right_bounds.max.x;
                        bounds.max.y = Max1F(left_bounds.max.y, right_bounds.max.y);
                    }
                    else
                    {
                        S8 pi = S8("∏{");
                        I2F text_bounds = VIS_BoundsFromS8(pi, cur);
                        R_CmdDrawS8(lisp_font, pi, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                        cur.x += text_bounds.max.x - text_bounds.min.x;
                        
                        bounds = text_bounds;
                        
                        while(0 != expr)
                        {
                            I2F child_bounds = VIS_ASTRender_(window, expr, cur, False);
                            cur.x += child_bounds.max.x - child_bounds.min.x;
                            expr = expr->next;
                            
                            if(0 != expr)
                            {
                                S8 comma = S8(", ");
                                I2F text_bounds = VIS_BoundsFromS8(comma, cur);
                                R_CmdDrawS8(lisp_font, comma, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                                cur.x += text_bounds.max.x - text_bounds.min.x;
                                bounds.min.x = Min1F(bounds.min.x, text_bounds.min.x);
                            }
                            else
                            {
                                S8 bracket = S8("}");
                                I2F text_bounds = VIS_BoundsFromS8(bracket, cur);
                                R_CmdDrawS8(lisp_font, bracket, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                                cur.x += text_bounds.max.x - text_bounds.min.x;
                                bounds.max.x = Max1F(bounds.max.x, text_bounds.max.x);
                            }
                            bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                            bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                        }
                    }
                } break;
                
                case(RT_OpKind_Divide):
                {
                    // TODO(tbt): more than two args
                    AST_Node *num = expr;
                    AST_Node *den = expr->next;
                    
                    V2F num_dim = Dimensions2F(VIS_ASTRender_(window, num, U2F(0.0f), True));
                    V2F den_dim = Dimensions2F(VIS_ASTRender_(window, den, U2F(0.0f), True));
                    
                    float total_height = num_dim.y + den_dim.y + 2.0f;
                    I2F num_bounds = VIS_ASTRender_(window, num, Add2F(cur, V2F(Max1F(0.0f, 0.5f*(den_dim.x - num_dim.x)), -0.5f*total_height)), False);
                    I2F den_bounds = VIS_ASTRender_(window, den, Add2F(cur, V2F(Max1F(0.0f, 0.5f*(num_dim.x - den_dim.x)), +0.5f*total_height)), False);
                    R_CmdDrawRectFill(RectMake2F(V2F(cur.x, 0.5f*(num_bounds.max.y + den_bounds.min.y)), V2F(Max1F(num_dim.x, den_dim.x), 2.0f)), Col(0.0f, 0.0f, 0.0f, 1.0f));
                    
                    bounds.min.x = Min1F(num_bounds.min.x, den_bounds.min.x);
                    bounds.min.y = Min1F(num_bounds.min.y, den_bounds.min.y);
                    bounds.max.x = Max1F(num_bounds.max.x, den_bounds.max.x);
                    bounds.max.y = Max1F(num_bounds.max.y, den_bounds.max.y);
                } break;
                
                case(RT_OpKind_Call):
                {
                    S8 string = S8FromFmt(frame_arena, "%.*s(", FmtS8(expr->value));
                    R_MeasuredText mt = R_MeasureS8(scratch.arena, lisp_font, string, cur);
                    R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                    cur.x += mt.bounds.max.x - mt.bounds.min.x;
                    
                    bounds = mt.bounds;
                    
                    AST_Node *args = expr->next;
                    while(0 != args)
                    {
                        I2F child_bounds = VIS_ASTRender_(window, args, cur, False);
                        cur.x += child_bounds.max.x - child_bounds.min.x;
                        args = args->next;
                        
                        if(0 != args)
                        {
                            S8 comma = S8(", ");
                            I2F text_bounds = VIS_BoundsFromS8(comma, cur);
                            R_CmdDrawS8(lisp_font, comma, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                            cur.x += text_bounds.max.x - text_bounds.min.x;
                            bounds.min.x = Min1F(bounds.min.x, text_bounds.min.x);
                        }
                        else
                        {
                            S8 bracket = S8(")");
                            I2F text_bounds = VIS_BoundsFromS8(bracket, cur);
                            R_CmdDrawS8(lisp_font, bracket, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                            cur.x += text_bounds.max.x - text_bounds.min.x;
                            bounds.max.x = Max1F(bounds.max.x, text_bounds.max.x);
                        }
                        bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                        bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                    }
                } break;
                
                case(RT_OpKind_Lambda):
                {
                    AST_Node *params = expr->children;
                    expr = expr->next;
                    
                    S8 lambda = S8("λ(");
                    if(0 == params)
                    {
                        lambda = S8("λ() = ");
                    }
                    I2F text_bounds = VIS_BoundsFromS8(lambda, cur);
                    R_CmdDrawS8(lisp_font, lambda, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                    cur.x += text_bounds.max.x - text_bounds.min.x;
                    
                    bounds = text_bounds;
                    
                    while(0 != params)
                    {
                        I2F child_bounds = VIS_ASTRender_(window, params, cur, False);
                        cur.x += child_bounds.max.x - child_bounds.min.x;
                        params = params->next;
                        
                        if(0 != params)
                        {
                            S8 comma = S8(", ");
                            I2F text_bounds = VIS_BoundsFromS8(comma, cur);
                            R_CmdDrawS8(lisp_font, comma, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                            cur.x += text_bounds.max.x - text_bounds.min.x;
                            bounds.min.x = Min1F(bounds.min.x, text_bounds.min.x);
                        }
                        else
                        {
                            S8 bracket = S8(") = ");
                            I2F text_bounds = VIS_BoundsFromS8(bracket, cur);
                            R_CmdDrawS8(lisp_font, bracket, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                            cur.x += text_bounds.max.x - text_bounds.min.x;
                            bounds.max.x = Max1F(bounds.max.x, text_bounds.max.x);
                        }
                        bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                        bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                    }
                    
                    cur.x = bounds.max.x;
                    I2F body_bounds = VIS_ASTRender_(window, expr, cur, False);
                    bounds.min.x = Min1F(bounds.min.x, body_bounds.min.x);
                    bounds.min.y = Min1F(bounds.min.y, body_bounds.min.y);
                    bounds.max.x = Max1F(bounds.max.x, body_bounds.max.x);
                    bounds.max.y = Max1F(bounds.max.y, body_bounds.max.y);
                } break;
                
                case(RT_OpKind_While):
                {
                    S8 string = S8("while ");
                    I2F text_bounds = VIS_BoundsFromS8(string, cur);
                    R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                    cur.x += text_bounds.max.x - text_bounds.min.x;
                    bounds = text_bounds;
                    
                    I2F cond_bounds = VIS_ASTRender_(window, expr, cur, False);
                    cur.x += cond_bounds.max.x - cond_bounds.min.x;
                    expr = expr->next;
                    
                    bounds.min.y = Min1F(bounds.min.y, cond_bounds.min.y);
                    bounds.max.x = Max1F(bounds.max.x, cond_bounds.max.x);
                    bounds.max.y = Max1F(bounds.max.y, cond_bounds.max.y);
                    cur.x = bounds.max.x + 16.0f;
                    
                    I2F body_bounds = VIS_ASTRender_(window, expr, cur, False);
                    bounds.min.x = Min1F(bounds.min.x, body_bounds.min.x);
                    bounds.min.y = Min1F(bounds.min.y, body_bounds.min.y);
                    bounds.max.x = Max1F(bounds.max.x, body_bounds.max.x);
                    bounds.max.y = Max1F(bounds.max.y, body_bounds.max.y);
                } break;
                
                case(RT_OpKind_If):
                {
                    // TODO(tbt): less broken layouting
                    
                    Bool start_x = cur.x;
                    
                    S8 string = S8("if ");
                    I2F text_bounds = VIS_BoundsFromS8(string, cur);
                    R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                    cur.x += text_bounds.max.x - text_bounds.min.x;
                    bounds = text_bounds;
                    
                    I2F cond_bounds = VIS_ASTRender_(window, expr, cur, False);
                    cur.x += cond_bounds.max.x - cond_bounds.min.x;
                    expr = expr->next;
                    
                    bounds.min.y = Min1F(bounds.min.y, cond_bounds.min.y);
                    bounds.max.x = Max1F(bounds.max.x, cond_bounds.max.x);
                    bounds.max.y = Max1F(bounds.max.y, cond_bounds.max.y);
                    cur.x = bounds.max.x + 16.0f;
                    
                    I2F body_bounds = VIS_ASTRender_(window, expr, cur, False);
                    bounds.min.x = Min1F(bounds.min.x, body_bounds.min.x);
                    bounds.min.y = Min1F(bounds.min.y, body_bounds.min.y);
                    bounds.max.x = Max1F(bounds.max.x, body_bounds.max.x);
                    bounds.max.y = Max1F(bounds.max.y, body_bounds.max.y);
                    
                    expr = expr->next;
                    if(0 != expr)
                    {
                        cur.x = start_x;
                        cur.y = bounds.max.y + lisp_font->v_advance;
                        
                        S8 string = S8("else ");
                        I2F text_bounds = VIS_BoundsFromS8(string, cur);
                        R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0));
                        cur.x += text_bounds.max.x - text_bounds.min.x;
                        bounds = text_bounds;
                        
                        I2F body_bounds = VIS_ASTRender_(window, expr, cur, False);
                        bounds.min.x = Min1F(bounds.min.x, body_bounds.min.x);
                        bounds.min.y = Min1F(bounds.min.y, body_bounds.min.y);
                        bounds.max.x = Max1F(bounds.max.x, body_bounds.max.x);
                        bounds.max.y = Max1F(bounds.max.y, body_bounds.max.y);
                    }
                } break;
                
                case(RT_OpKind_LT):
                case(RT_OpKind_GT):
                case(RT_OpKind_LTE):
                case(RT_OpKind_GTE):
                case(RT_OpKind_Eq):
                case(RT_OpKind_NEq):
                case(RT_OpKind_Modulo):
                case(RT_OpKind_Or):
                case(RT_OpKind_And):
                {
                    I2F left_bounds = VIS_ASTRender_(window, expr, cur, False);
                    cur.x += left_bounds.max.x - left_bounds.min.x;
                    expr = expr->next;
                    
                    S8 string = VIS_S8FromBinOp(op_kind);
                    I2F text_bounds = VIS_BoundsFromS8(string, cur);
                    R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0f));
                    cur.x += text_bounds.max.x - text_bounds.min.x;
                    
                    I2F right_bounds = VIS_ASTRender_(window, expr, cur, False);
                    cur.x += right_bounds.max.x - right_bounds.min.x;
                    expr = expr->next;
                    
                    bounds.min.x = left_bounds.min.x;
                    bounds.min.y = Min1F(left_bounds.min.y, right_bounds.min.y);
                    bounds.max.x = right_bounds.max.x;
                    bounds.max.y = Max1F(left_bounds.max.y, right_bounds.max.y);
                } break;
                
                case(RT_OpKind_Rect):
                case(RT_OpKind_Circle):
                case(RT_OpKind_Col):
                {
                    bounds = I2F(V2F(Infinity, Infinity), V2F(NegativeInfinity, NegativeInfinity));
                    
                    S8 string = VIS_S8FromFn(op_kind);
                    I2F text_bounds = VIS_BoundsFromS8(string, cur);
                    R_CmdDrawS8(lisp_font, string, cur, Col(0.0f, 0.0f, 0.0f, 1.0f));
                    cur.x += text_bounds.max.x - text_bounds.min.x;
                    bounds.min.x = Min1F(bounds.min.x, text_bounds.min.x);
                    bounds.max.x = Max1F(bounds.max.x, text_bounds.max.x);
                    bounds.min.y = Min1F(bounds.min.y, text_bounds.min.y);
                    bounds.max.y = Max1F(bounds.max.y, text_bounds.max.y);
                    
                    while(0 != expr)
                    {
                        I2F child_bounds = VIS_ASTRender_(window, expr, cur, False);
                        cur.x += child_bounds.max.x - child_bounds.min.x + 12.0f;
                        expr = expr->next;
                        
                        bounds.min.x = Min1F(bounds.min.x, child_bounds.min.x);
                        bounds.max.x = Max1F(bounds.max.x, child_bounds.max.x);
                        bounds.min.y = Min1F(bounds.min.y, child_bounds.min.y);
                        bounds.max.y = Max1F(bounds.max.y, child_bounds.max.y);
                    }
                } break;
            }
        }
        else
        {
            if(0 == ast->value.len)
            {
                bounds = VIS_ASTRender_(window, ast->children, cur, False);
            }
            else
            {
                R_MeasuredText mt = R_MeasureS8(scratch.arena, lisp_font, ast->value, cur);
                bounds = mt.bounds;
                R_CmdDrawS8(lisp_font, ast->value, cur, Col(0.0f, 0.0f, 0.0f, 1.0f));
                
                if(IntervalHasValue2F(bounds, mouse_pos))
                {
                    float val = S8Parse1F(ast->value);
                    int scroll = EV_QueueScrollGet(events).y;
                    if(0 != scroll && !IsNaN1F(val))
                    {
                        float percentage = Max1F(Abs1F(0.1f*val), 0.01f);
                        val += scroll * percentage;
                        
                        M_FreeListFree(&lisp_allocator, ast->value.buffer);
                        
                        S8 new_val = S8FromFmt(scratch.arena, "%f", val);
                        while(new_val.len > 1 && '0' == new_val.buffer[new_val.len - 1])
                        {
                            new_val.len -= 1;
                        }
                        if(new_val.len > 1 && '.' == new_val.buffer[new_val.len - 1])
                        {
                            new_val.len -= 1;
                        }
                        
                        ast->value = S8CloneFL(&lisp_allocator, new_val);
                        
                        lisp_is_ast_dirty = True;
                    }
                }
            }
        }
    }
    
    if(measure_only)
    {
        R_CmdQueueRecordingEnd(&dummy);
    }
    else
    {
        if(IntervalHasValue2F(bounds, mouse_pos))
        {
            R_CmdDrawRectStroke(bounds, 2.0f, Col(0.2f, 0.6f, 0.4f, 1.0f));
            R_CmdDrawRectFill(bounds, Col(0.2f, 0.6f, 0.4f, 0.4f));
            if(0 == vis_tooltip.len)
            {
                vis_tooltip = ast->value;
                if(0 == vis_tooltip.buffer)
                {
                    S8 source = AST_S8FromTree(scratch.arena, ast, False);
                    S8 value = RT_S8FromVal(scratch.arena, ast->evaluated);
                    vis_tooltip = S8FromFmt(frame_arena, "%.*s = %.*s", FmtS8(source), FmtS8(value));
                }
            }
            
            if(EV_QueueHasKeyDown(events, I_Key_MouseButtonLeft, 0, True))
            {
                
                ed_editing = ast;
                S8 str = AST_S8FromTree(scratch.arena, ast, False);
                M_Set(ed_backing, 0, sizeof(ed_backing));
                M_Copy(ed_backing, str.buffer, Min1U(str.len, sizeof(ed_backing) - 1));
                ed_selection.mark = 0;
                ed_selection.cursor = str.len;
            }
        }
        
        if(RT_ValKind_Colour == ast->evaluated.kind)
        {
            R_LayerRelative(-1) R_CmdDrawRoundedRect(Expand2F(bounds, U2F(2.0f)), 2.0f, ast->evaluated.col);
        }
    }
    
    M_TempEnd(&scratch);
    return bounds;
}

I2F vis_root_bounds;

Function void
VIS_ASTRender(W_Handle window, AST_Node *root, V2F pos)
{
    vis_tooltip = (S8){0};
    vis_root_bounds = VIS_ASTRender_(window, root, pos, False);
    if(vis_tooltip.len > 0 && 0 == ed_editing)
    {
        V2F mouse_pos = W_MousePositionGet(window);
        R_MeasuredText mt = R_MeasureS8(W_FrameArenaGet(window), lisp_tooltip_font, vis_tooltip, mouse_pos);
        R_LayerRelative(+1) R_CmdDrawRoundedRect(Expand2F(mt.bounds, U2F(4.0f)), 4.0f, Col(0.2f, 0.6f, 0.4f, 1.0f));
        R_LayerRelative(+2) R_CmdDrawS8(lisp_tooltip_font, mt.string, mouse_pos, U4F(1.0f));
    }
}
