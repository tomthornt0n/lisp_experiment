
#include "lisp__ui.h"
#include "lisp__runtime.h"
#include "lisp__ast.h"
#include "lisp__visual.h"
#include "lisp__editor.h"

Function void LISP_Init            (W_Handle window);
Function void LISP_UpdateAndRender (W_Handle window);
Function void LISP_Cleanup         (W_Handle window);

Global G_AppHooks lisp_app_hooks =
{
    LISP_Init,
    LISP_UpdateAndRender,
    LISP_Cleanup,
};

Global M_FreeList lisp_allocator;

Global V2F lisp_camera;

Global R_Font *lisp_font;
Global R_Font *lisp_tooltip_font;
Global R_Font *lisp_editor_font;

Global Bool lisp_is_ast_dirty = True;
Global AST_Node *lisp_ast;
