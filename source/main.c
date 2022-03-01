
//~NOTE(tbt): includes

//-NOTE(tbt): base layer
#include "base/base.h"
#include "base/base.c"

//-NOTE(tbt): os layer
#include "os/os.h"
#include "os/os.c"

//-NOTE(tbt): graphics layer
#include "graphics/graphics.h"
#include "graphics/graphics.c"

//-NOTE(tbt): collections
#include "collections/collections.c"

//-NOTE(tbt): app layer
#include "lisp/lisp.h"
#include "lisp/lisp.c"

//~NOTE(tbt): entry point

#if Build_NoCRT
# define EntryPoint Function int APP_EntryPoint(void)
#else
# define EntryPoint int main(int argc, char **argv)
#endif

EntryPoint
{
    G_Init();
    G_WindowOpen(S8("lispy"), V2I(1024, 768), lisp_app_hooks);
    G_MainLoop();
    G_Cleanup();
    return 0;
}
