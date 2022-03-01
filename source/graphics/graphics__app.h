
typedef void G_WindowOpenHook(W_Handle window);
typedef void G_WindowCloseHook(W_Handle window);

typedef struct
{
    G_WindowOpenHook *open;
    W_DrawHook *draw;
    G_WindowCloseHook *close;
} G_AppHooks;
#define G_AppHooks(O, D, C) ((G_AppHooks){ (O), (D), (C), })

Function void     G_Init        (void);
Function W_Handle G_WindowOpen  (S8 title, V2I dimensions, G_AppHooks hooks);
Function void     G_WindowClose (W_Handle window);
Function void     G_MainLoop    (void);
Function void     G_Cleanup     (void);

Function W_Handle G_WindowsFirstGet (void);
Function W_Handle G_WindowsNextGet  (W_Handle current);
#define G_WindowsForEach(V) W_Handle (V) = G_WindowsFirstGet(); 0 != (V); (V) = G_WindowsNextGet(V)

Function Bool G_ShouldBlockToWaitForEventsGet (void);
Function void G_ShouldBlockToWaitForEventsSet (Bool should_block_to_wait_for_events);
Function void G_ForceNextUpdate               (void); // NOTE(tbt): call to temporarily override G_ShouldBlockToWaitForEvents() to False for one frame

Function W_Handle G_CurrentWindowGet (void);            // NOTE(tbt): retrieve the current window being updated from G_MainLoop
Function M_Arena *G_ArenaFromWindow  (W_Handle window); // NOTE(tbt): get an arena with the lifetime of a window created with G_WindowOpen

