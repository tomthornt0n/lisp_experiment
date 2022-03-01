
typedef struct W32_WindowNode W32_WindowNode;
struct W32_WindowNode
{
    W32_Window w;
    
    M_Arena arena;
    W32_WindowNode *next;
    W32_WindowNode *prev;
    G_WindowCloseHook *cleanup;
};

typedef struct
{
    HANDLE instance_handle;
    ID3D11Device *device;
    ID3D11DeviceContext *device_ctx;
    
    W32_WindowNode *windows_first;
    W32_WindowNode *windows_last;
    
    W32_WindowNode *current_window;
    
    volatile Bool should_block_to_wait_for_events;
    volatile Bool should_force_next_update;
} W32_GraphicalApp;
Global W32_GraphicalApp w32_g_app = {0};

#define W32_WindowClassName ApplicationNameWString L"__WindowClass"

Function void W32_WindowNodeRemove (W32_WindowNode *w);
