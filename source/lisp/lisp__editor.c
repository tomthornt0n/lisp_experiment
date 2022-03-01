
Global AST_Node *ed_ast = 0;

Function void
ED_Edit(W_Handle window)
{
    if(0 != ed_editing)
    {
        if(EV_QueueHasKeyDown(W_EventQueueGet(window), I_Key_Esc, 0, True))
        {
            ed_editing = 0;
        }
        else if(EV_QueueHasKeyDown(W_EventQueueGet(window), I_Key_Enter, 0, True) || 0 == CStringCalculateUTF8Length(ed_backing))
        {
            AST_Node *new_tree = AST_TreeFromS8(&lisp_allocator, CStringAsS8(ed_backing));
            if(0 == new_tree)
            {
                if(0 != ed_editing->next)
                {
                    ed_editing->next->from = ed_editing->from;
                }
                *ed_editing->from = ed_editing->next;
                AST_TreeFree(&lisp_allocator, ed_editing);
            }
            else
            {
                AST_Node *tail = new_tree;
                while(0 != tail->next)
                {
                    tail = tail->next;
                }
                tail->next = ed_editing->next;
                if(0 != tail->next)
                {
                    tail->next->from = &tail->next;
                }
                new_tree->from = ed_editing->from;
                *ed_editing->from = new_tree;
                AST_TreeFree(&lisp_allocator, ed_editing);
            }
            lisp_is_ast_dirty = True;
            ed_editing = 0;
        }
        else
        {
            UI_EditText(ed_backing, sizeof(ed_backing), &ed_selection, 0, True, 0, 0);
        }
    }
}
