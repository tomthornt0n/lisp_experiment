
Function xcb_intern_atom_reply_t *LINUX_XAtomReplyGet (char *atom_id);

Global struct
{
#define LINUX_GlProcDef(TYPE, FUNCTION) PFNGL ## TYPE ## PROC FUNCTION;
#include "linux_graphics__gl_proc_list.h"
} gl;

Global struct
{
    PFNGLXCHOOSEFBCONFIGPROC        ChooseFBConfig;
    PFNGLXCREATENEWCONTEXTPROC      CreateNewContext;
    PFNGLXCREATEWINDOWPROC          CreateWindow;
    PFNGLXDESTROYCONTEXTPROC        DestroyContext;
    PFNGLXDESTROYWINDOWPROC         DestroyWindow;
    PFNGLXGETFBCONFIGATTRIBPROC     GetFBConfigAttrib;
    PFNGLXMAKECONTEXTCURRENTPROC    MakeContextCurrent;
    PFNGLXQUERYEXTENSIONSSTRINGPROC QueryExtensionsString;
    PFNGLXSWAPBUFFERSPROC           SwapBuffers;
    PFNGLXSWAPINTERVALEXTPROC       SwapIntervalEXT;
    PFNGLXSWAPINTERVALMESAPROC      SwapIntervalMESA;
    PFNGLXSWAPINTERVALSGIPROC       SwapIntervalSGI;
} glX;

typedef void LINUX_GLProc(void);

Function Bool          LINUX_GLHasExtension (int screen, char *extension);
Function LINUX_GLProc *LINUX_GLProcLoad     (char *function);

typedef struct LINUX_WindowNode LINUX_WindowNode;
struct LINUX_WindowNode
{
    LINUX_Window w;
    
    M_Arena arena;
    LINUX_WindowNode *next;
    LINUX_WindowNode *prev;
    G_WindowCloseHook *cleanup;
};

typedef struct
{
    GLXContext context;
    Display *display;
    xcb_connection_t *connection;
    int default_screen_id;
    xcb_screen_t *screen;
    
    int visual_id;
    unsigned int value_list[3];
    GLXFBConfig framebuffer_config;
    xcb_intern_atom_reply_t *wm_protocol_reply;
    xcb_intern_atom_reply_t *window_close_reply;
    
    LINUX_WindowNode *windows_first;
    LINUX_WindowNode *windows_last;
    
    LINUX_WindowNode *current_window;
    
    volatile Bool should_block_to_wait_for_events;
    volatile Bool should_force_next_update;
} LINUX_GraphicalApp;
Global LINUX_GraphicalApp linux_g_app = {0};

Function void LINUX_WindowNodeRemove (LINUX_WindowNode *w);
