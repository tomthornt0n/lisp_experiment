
typedef struct W32_Window W32_Window;
struct W32_Window
{
    M_Arena frame_arena;
    
    HWND window_handle;
    IDXGISwapChain1 *swap_chain;
    ID3D11RenderTargetView *backbuffer_render_target_view;
    ID3D11Resource *backbuffer_render_target_texture;
    ID3D11RenderTargetView *render_target_view;
    ID3D11Resource *render_target_texture;
    ID3D11DepthStencilView *depth_stencil_view;
    
    Bool should_close;
    EV_Queue events;
    
    R_CmdQueue r_cmd_queue;
    W_DrawHook *draw;
    
    void *user_data;
    
    double frame_start;
    double frame_end;
    
    Bool is_vsync;
    W_CursorKind cursor_kind;
    
    WINDOWPLACEMENT prev_window_placement;
    
    M4x4F projection_matrix;
    
    V2I dimensions;
    V2F mouse_position;
    
    Bool is_key_down[I_Key_MAX];
};

Function void W32_WindowMake    (W32_Window *window, S8 title, V2I dimensions, W_DrawHook draw);
Function void W32_WindowDestroy (W32_Window *window);

Function LRESULT W32_WindowProc (HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);
Function void    W32_WindowDraw (W32_Window *w);
