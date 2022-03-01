
typedef struct
{
    xcb_generic_event_t *prev;
    xcb_generic_event_t *current;
    xcb_generic_event_t *next;
} LINUX_EventQueue;

Function void LINUX_EventQueueUpdate (LINUX_EventQueue *queue);

typedef struct LINUX_Window LINUX_Window;
struct LINUX_Window
{
    M_Arena frame_arena;
    
    xcb_window_t window;
    GLXDrawable drawable;
    
    Bool should_close;
    LINUX_EventQueue event_queue;
    EV_Queue events;
    
    R_CmdQueue r_cmd_queue;
    W_DrawHook *draw;
    
    void *user_data;
    
    double frame_start;
    double frame_end;
    
    M4x4F projection_matrix;
    
    V2I dimensions;
    V2F mouse_position;
    
    Bool is_key_down[I_Key_MAX];
};

Function void LINUX_WindowMake    (LINUX_Window *window, S8 title, V2I dimensions, W_DrawHook draw);
Function void LINUX_WindowDraw    (LINUX_Window *window);
Function void LINUX_WindowDestroy (LINUX_Window *window);
