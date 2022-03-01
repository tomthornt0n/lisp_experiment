
#include "lisp__ui.c"
#include "lisp__ast.c"
#include "lisp__runtime.c"
#include "lisp__visual.c"
#include "lisp__editor.c"

#include "external/NotoSerif-Regular.c"
#include "external/share_tech_mono.c"

Global Bool lisp_is_dragging;
Global V2F lisp_drag_mouse;
Global V2F lisp_drag_cam;

Global S8 lisp_save_path;

Function void
LISP_Init(W_Handle window)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    G_ShouldBlockToWaitForEventsSet(False);
    W_VSyncSet(window, False);
    lisp_font = R_FontMake(S8(lisp_font_data), 32);
    lisp_tooltip_font = R_FontMake(S8(lisp_code_font_data), 16);
    lisp_editor_font = R_FontMake(S8(lisp_code_font_data), 28);
    lisp_allocator = M_FreeListMake(m_default_hooks, M_FreeListPlacementPolicy_First);
    rt_global_env = RT_EnvMake(256, 0);
    vis_layout_cache = VIS_LayoutCacheMake(64);
    
    lisp_save_path = FilenamePush(G_ArenaFromWindow(window), F_StdPathGet(scratch.arena, F_StdPath_Config), S8("VisualLispWorkspace.txt"));
    S8 data = F_ReadEntire(scratch.arena, lisp_save_path);
    if(0 == data.len)
    {
        data = S8("(begin (set! x 1))");
    }
    lisp_ast = AST_TreeFromS8(&lisp_allocator, data);
    lisp_ast->from = &lisp_ast;
    
    M_TempEnd(&scratch);
}

Global R_CmdQueue lisp_render_commands = {0};

Function void
LISP_UpdateAndRender(W_Handle window)
{
    EV_Queue *events = W_EventQueueGet(window);
    
    if(lisp_is_ast_dirty)
    {
        RT_ASTExec(lisp_ast, rt_global_env);
        lisp_is_ast_dirty = False;
    }
    
    R_Layer(0) R_CmdClear(U4F(1.0f));
    R_Layer(2) VIS_ASTRender(window, lisp_ast, lisp_camera);
    ED_Edit(window);
    R_CmdQueueRecordingBegin(&lisp_render_commands);
    RT_Val update = RT_EnvLookup(rt_global_env, S8("update"));
    if(RT_ValKind_Lambda == update.kind)
    {
        RT_UpdateKeyState(window, rt_global_env);
        RT_Env *child_env = RT_EnvMake(256, rt_global_env);
        RT_ASTExec(update.proc.expr, child_env);
        RT_VarTableDestroy(&child_env->variables);
    }
    R_CmdQueueRecordingEnd(&lisp_render_commands);
    R_Layer(1) R_CmdSubQueue(&lisp_render_commands);
    
    if(EV_QueueHasKeyDown(events, I_Key_MouseButtonLeft, 0, True))
    {
        lisp_drag_mouse = W_MousePositionGet(window);
        lisp_drag_cam = lisp_camera;
        lisp_is_dragging = True;
    }
    else if(EV_QueueHasKeyUp(events, I_Key_MouseButtonLeft, 0, True))
    {
        lisp_is_dragging = False;
    }
    if(lisp_is_dragging)
    {
        lisp_camera = Add2F(lisp_drag_cam, Sub2F(W_MousePositionGet(window), lisp_drag_mouse));
    }
}

Function void
LISP_Cleanup(W_Handle window)
{
    F_WriteEntire(lisp_save_path, AST_S8FromTree(G_ArenaFromWindow(window), lisp_ast, True));
}
