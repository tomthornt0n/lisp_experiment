
Function AST_Node *
AST_TreeFromS8_(M_FreeList *allocator, S8 *input, AST_Node *parent)
{
    AST_Node *first = 0;
    AST_Node *last = 0;
    
    while(input->len > 0)
    {
        while(input->len > 0 && CharIsSpace(input->buffer[0]))
        {
            S8Advance(input, 1);
        }
        
        if(S8Consume(input, S8(")")))
        {
            break;
        }
        
        AST_Node *node = M_FreeListAlloc(allocator, sizeof(*node));
        
        if(S8Consume(input, S8("(")))
        {
            node->children = AST_TreeFromS8_(allocator, input, node);
        }
        else
        {
            S8 value = {0};
            value.buffer = input->buffer;
            while(input->len > 0 && ')' != input->buffer[0] && !CharIsSpace(input->buffer[0]))
            {
                S8Advance(input, 1);
                value.len += 1;
            }
            node->value = S8CloneFL(allocator, value);
        }
        
        if(0 == last)
        {
            first = node;
            node->from = &parent->children;
        }
        else
        {
            last->next = node;
            node->from = &last->next;
        }
        last = node;
        
        if(0 != parent)
        {
            parent->children_count += 1;
        }
    }
    
    return first;
}

Function AST_Node *
AST_TreeFromS8(M_FreeList *allocator, S8 input)
{
    AST_Node *result = AST_TreeFromS8_(allocator, &input, 0);
    return result;
}

Function AST_Node *
AST_TreeClone(M_FreeList *allocator, AST_Node *root)
{
    AST_Node *result = 0;
    if(0 != root)
    {
        result = M_FreeListAlloc(allocator, sizeof(*result));
        result->value = S8CloneFL(allocator, root->value);
        result->evaluated = root->evaluated;
        result->children_count = root->children_count;
        result->next = AST_TreeClone(allocator, root->next);
        result->children = AST_TreeClone(allocator, root->children);
        if(0 != result->next)
        {
            result->next->from = &result->next;
        }
        if(0 != result->children)
        {
            result->children->from = &result->children;
        }
    }
    return result;
}

Function void
AST_TreeFree(M_FreeList *allocator, AST_Node *root)
{
    if(0 != root)
    {
        AST_Node *next = 0;
        for(AST_Node *n = root->children; 0 != n; n = next)
        {
            next = n->next;
            AST_TreeFree(allocator, n);
        }
        M_FreeListFree(allocator, root->value.buffer);
        M_FreeListFree(allocator, root);
    }
}

Function void
AST_S8FromTree_(M_Arena *arena, AST_Node *root, S8List *output)
{
    if(0 != root->children)
    {
        S8ListAppend(arena, output, S8("("));
        for(AST_Node *n = root->children; 0 != n; n = n->next)
        {
            AST_S8FromTree_(arena, n, output);
        }
        S8ListAppend(arena, output, S8(")"));
    }
    else if(0 != root->value.len)
    {
        S8ListAppend(arena, output, root->value);
    }
    else
    {
        S8ListAppend(arena, output, S8("()"));
    }
    if(0 != root->next)
    {
        S8ListAppend(arena, output, S8(" "));
    }
}

Function S8
AST_S8FromTree(M_Arena *arena, AST_Node *root, Bool include_siblings)
{
    S8List s = {0};
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    while(0 != root)
    {
        AST_S8FromTree_(scratch.arena, root, &s);
        root = root->next;
        if(!include_siblings)
        {
            break;
        }
    }
    S8 result = S8ListJoin(arena, s);
    M_TempEnd(&scratch);
    return result;
}
