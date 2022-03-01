
Function void
W32_WindowMake(W32_Window *window, S8 title, V2I dimensions, W_DrawHook draw)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    window->frame_arena = M_ArenaMake(m_default_hooks);
    window->events = EV_QueueMake();
    
    //-NOTE(tbt): create native window
    S16 title_s16 = S16FromS8(scratch.arena, title);
    DWORD window_styles_ex = (WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP);
    DWORD window_styles = (WS_OVERLAPPEDWINDOW);
    RECT window_rect = { 0, 0, dimensions.x, dimensions.y };
    AdjustWindowRect(&window_rect, window_styles, FALSE);
    
    window->window_handle = CreateWindowExW(window_styles_ex,
                                            W32_WindowClassName,
                                            title_s16.buffer,
                                            window_styles,
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            window_rect.right - window_rect.left,
                                            window_rect.bottom - window_rect.top,
                                            0, 0, w32_g_app.instance_handle,
                                            0);
    SetWindowLongPtrW(window->window_handle, GWLP_USERDATA, IntFromPtr(window));
    
    //-NOTE(tbt): setup D3D11 swapchain
    IDXGIFactory2 *dxgi_factory = 0;
    {
        IDXGIDevice *dxgi_device = 0;
        ID3D11Device_QueryInterface(w32_g_app.device, &IID_IDXGIDevice, &dxgi_device);
        IDXGIAdapter *dxgi_adapter = 0;
        IDXGIDevice_GetAdapter(dxgi_device, &dxgi_adapter);
        IDXGIAdapter_GetParent(dxgi_adapter, &IID_IDXGIFactory, &dxgi_factory);
        IDXGIAdapter_Release(dxgi_adapter);
        IDXGIDevice_Release(dxgi_device);
    }
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc =
    {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    };
    IDXGIFactory2_CreateSwapChainForHwnd(dxgi_factory,
                                         (IUnknown *)w32_g_app.device,
                                         window->window_handle,
                                         &swap_chain_desc,
                                         0, 0, &window->swap_chain);
    IDXGIFactory2_Release(dxgi_factory);
    
    //-NOTE(tbt): show native window
    ShowWindow(window->window_handle, SW_SHOW);
    UpdateWindow(window->window_handle);
    
    //-NOTE(tbt): save draw hook
    window->draw = draw;
    
    M_TempEnd(&scratch);
}

Function void
W32_WindowDestroy(W32_Window *window)
{
    IDXGISwapChain_Release(window->swap_chain);
    DestroyWindow(window->window_handle);
    M_ArenaDestroy(&window->frame_arena);
}

Function LRESULT
W32_WindowProc(HWND window_handle,
               UINT message,
               WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    
    W32_Window *w = PtrFromInt(GetWindowLongPtrW(window_handle, GWLP_USERDATA));
    
    Persist Bool is_mouse_hover_active = False;
    
    I_Modifiers modifiers = 0;
    if(GetKeyState(VK_CONTROL) & 0x8000)
    {
        modifiers |= I_Modifiers_Ctrl;
    }
    if(GetKeyState(VK_SHIFT) & 0x8000)
    {
        modifiers |= I_Modifiers_Shift;
    }
    if(GetKeyState(VK_MENU) & 0x8000)
    {
        modifiers |= I_Modifiers_Alt;
    }
    
    switch(message)
    {
        case(WM_CLOSE):
        case(WM_QUIT):
        case(WM_DESTROY):
        {
            w->should_close = True;
        } break;
        
        case(WM_LBUTTONDOWN):
        case(WM_LBUTTONUP):
        {
            EV_Data event =
            {
                .kind = EV_Kind_Key,
                .modifiers = modifiers,
                .key = I_Key_MouseButtonLeft,
                .is_down = (WM_LBUTTONDOWN == message),
                .position = W_MousePositionGet(w),
            };
            EV_QueuePush(&w->events, event);
            w->is_key_down[event.key] = event.is_down;
        } break;
        
        case(WM_MBUTTONDOWN):
        case(WM_MBUTTONUP):
        {
            EV_Data event =
            {
                .kind = EV_Kind_Key,
                .modifiers = modifiers,
                .key = I_Key_MouseButtonMiddle,
                .is_down = (WM_MBUTTONDOWN == message),
            };
            EV_QueuePush(&w->events, event);
            w->is_key_down[event.key] = event.is_down;
        } break;
        
        case(WM_RBUTTONDOWN):
        case(WM_RBUTTONUP):
        {
            EV_Data event =
            {
                .kind = EV_Kind_Key,
                .modifiers = modifiers,
                .key = I_Key_MouseButtonRight,
                .is_down = (WM_RBUTTONDOWN == message),
            };
            EV_QueuePush(&w->events, event);
            w->is_key_down[event.key] = event.is_down;
        } break;
        
        case(WM_XBUTTONDOWN):
        case(WM_XBUTTONUP):
        {
            UINT button = GET_XBUTTON_WPARAM(w_param);
            EV_Data event =
            {
                .kind = EV_Kind_Key,
                .modifiers = modifiers,
                .key = (XBUTTON1 == button) ? I_Key_MouseButtonForward : I_Key_MouseButtonBackward,
                .is_down = (WM_XBUTTONDOWN == message),
            };
            EV_QueuePush(&w->events, event);
            w->is_key_down[event.key] = event.is_down;
        } break;
        
        case(WM_MOUSEMOVE):
        {
            EV_Data event =
            {
                .kind = EV_Kind_MouseMove,
                .modifiers = modifiers,
                .position = W_MousePositionGet(w),
            };
            V2F delta = Sub2F(event.position, w->mouse_position);
            event.size = V2I(delta.x + 0.5f, delta.y + 0.5f);
            EV_QueuePush(&w->events, event);
            
            w->mouse_position = event.position;
            
            if(!is_mouse_hover_active)
            {
                is_mouse_hover_active = True;
                TRACKMOUSEEVENT track_mouse_event =
                {
                    .cbSize = sizeof(track_mouse_event),
                    .dwFlags = TME_LEAVE,
                    .hwndTrack = window_handle,
                    .dwHoverTime = HOVER_DEFAULT,
                };
                TrackMouseEvent(&track_mouse_event);
            }
        } break;
        
        case(WM_MOUSELEAVE):
        {
            is_mouse_hover_active = False;
            SetCursor(LoadCursorW(0, IDC_ARROW));
            EV_Data event =
            {
                .kind = EV_Kind_MouseLeave,
                .modifiers = modifiers,
            };
            EV_QueuePush(&w->events, event);
            M_Set(w->is_key_down, False, sizeof(w->is_key_down));
        } break;
        
        case(WM_MOUSEWHEEL):
        {
            V2F delta = V2F(0.0f, GET_WHEEL_DELTA_WPARAM(w_param) / 120.0f);
            EV_Data event =
            {
                .kind = EV_Kind_MouseScroll,
                .modifiers = modifiers,
                .position = delta,
            };
            EV_QueuePush(&w->events, event);
        } break;
        
        case(WM_MOUSEHWHEEL):
        {
            V2F delta = V2F(GET_WHEEL_DELTA_WPARAM(w_param) / 120.0f, 0.0f);
            EV_Data event =
            {
                .kind = EV_Kind_MouseScroll,
                .modifiers = modifiers,
                .position = delta,
            };
            EV_QueuePush(&w->events, event);
        } break;
        
        case(WM_SETCURSOR):
        {
            V2I window_dimensions = W_DimensionsGet(w);
            V2F mouse_position = W_MousePositionGet(w);
            
            Bool should_hide_cursor = False;
            
            if(is_mouse_hover_active &&
               mouse_position.x > 0 &&
               mouse_position.x < window_dimensions.x &&
               mouse_position.y > 0 &&
               mouse_position.y < window_dimensions.y)
            {
                switch(w->cursor_kind)
                {
                    default: break;
                    
                    case(W_CursorKind_HResize):
                    {
                        SetCursor(LoadCursorW(0, IDC_SIZEWE));
                    } break;
                    
                    case(W_CursorKind_VResize):
                    {
                        SetCursor(LoadCursorW(0, IDC_SIZENS));
                    } break;
                    
                    case(W_CursorKind_Default):
                    {
                        SetCursor(LoadCursorW(0, IDC_ARROW));
                    } break;
                    
                    case(W_CursorKind_Hidden):
                    {
                        should_hide_cursor = True;
                    } break;
                }
            }
            else
            {
                result = DefWindowProc(window_handle, message, w_param, l_param);
            }
            
            if(should_hide_cursor)
            {
                while(ShowCursor(FALSE) >= 0);
            }
            else
            {
                while(ShowCursor(TRUE) < 0);
            }
        } break;
        
        case(WM_SIZE):
        {
            V2I window_dimensions = V2I(LOWORD(l_param), HIWORD(l_param));
            
            EV_Data event =
            {
                .kind = EV_Kind_WindowSize,
                .modifiers = modifiers,
                .size = window_dimensions,
            };
            EV_QueuePush(&w->events, event);
            w->dimensions = window_dimensions;
            
            if(0 != w->backbuffer_render_target_view)
            {
                ID3D11DeviceContext_ClearState(w32_g_app.device_ctx);
                ID3D11RenderTargetView_Release(w->backbuffer_render_target_view);
                ID3D11RenderTargetView_Release(w->render_target_view);
                ID3D11RenderTargetView_Release(w->depth_stencil_view);
                ID3D11Resource_Release(w->backbuffer_render_target_texture);
                ID3D11Resource_Release(w->render_target_texture);
                w->backbuffer_render_target_view = 0;
                w->render_target_view = 0;
                w->depth_stencil_view = 0;
                w->backbuffer_render_target_texture = 0;
                w->render_target_texture = 0;
            }
            if(window_dimensions.x > 0 &&
               window_dimensions.y > 0)
            {
                //-NOTE(tbt): resize swapchain
                IDXGISwapChain_ResizeBuffers(w->swap_chain, 0,
                                             window_dimensions.x,
                                             window_dimensions.y,
                                             DXGI_FORMAT_UNKNOWN, 0);
                
                //-NOTE(tbt): recreate MSAA render target
                D3D11_RENDER_TARGET_VIEW_DESC render_target_desc =
                {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS,
                };
                D3D11_TEXTURE2D_DESC render_target_texture_desc =
                {
                    .Width = window_dimensions.x,
                    .Height = window_dimensions.y,
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc =
                    {
                        .Count = 2,
                        .Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN,
                    },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_RENDER_TARGET,
                };
                ID3D11Texture2D *render_target;
                ID3D11Device_CreateTexture2D(w32_g_app.device,
                                             &render_target_texture_desc,
                                             0, &render_target);
                w->render_target_texture = (ID3D11Resource *)render_target;
                ID3D11Device_CreateRenderTargetView(w32_g_app.device,
                                                    w->render_target_texture,
                                                    &render_target_desc,
                                                    &w->render_target_view);
                
                //-NOTE(tbt): recreate backbuffer render target
                D3D11_RENDER_TARGET_VIEW_DESC backbuffer_render_target_desc =
                {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
                };
                ID3D11Texture2D *backbuffer_render_target;
                IDXGISwapChain_GetBuffer(w->swap_chain, 0,
                                         &IID_ID3D11Texture2D,
                                         (void **)&backbuffer_render_target);
                w->backbuffer_render_target_texture = (ID3D11Resource*)backbuffer_render_target;
                ID3D11Device_CreateRenderTargetView(w32_g_app.device,
                                                    w->backbuffer_render_target_texture,
                                                    &backbuffer_render_target_desc,
                                                    &w->backbuffer_render_target_view);
                
                //-NOTE(tbt): recreate depth stencil target
                D3D11_TEXTURE2D_DESC depth_stencil_texture_desc =
                {
                    .Width = window_dimensions.x,
                    .Height = window_dimensions.y,
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                    .SampleDesc =
                    {
                        .Count = 2,
                        .Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN,
                    },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                };
                ID3D11Texture2D *depth_stencil;
                ID3D11Device_CreateTexture2D(w32_g_app.device,
                                             &depth_stencil_texture_desc, 0,
                                             &depth_stencil);
                ID3D11Device_CreateDepthStencilView(w32_g_app.device,
                                                    (ID3D11Resource*)depth_stencil,
                                                    0, &w->depth_stencil_view);
                ID3D11Texture2D_Release(depth_stencil);
                
                //-NOTE(tbt): projection
                w->projection_matrix = OrthoMake4x4F(0.0f, window_dimensions.x, 0.0f, window_dimensions.y, 0.0f, 1.0f);
            }
        } break;
        
        case(WM_PAINT):
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(window_handle, &ps);
            W32_WindowDraw(w);
            IDXGISwapChain_Present(w->swap_chain, 0, 0);
            EndPaint(window_handle, &ps);
        } break;
        
        case(WM_SYSKEYDOWN):
        case(WM_SYSKEYUP):
        case(WM_KEYDOWN):
        case(WM_KEYUP):
        {
            size_t vk = w_param;
            Bool was_down = !!(l_param & Bit(30));
            Bool is_down = !(l_param & Bit(31));
            
            I_Key key_input = I_Key_None;
            if((vk >= 'A' && vk <= 'Z') ||
               (vk >= '0' && vk <= '9'))
            {
                key_input = (vk >= 'A' && vk <= 'Z') ? I_Key_A + (vk - 'A') : I_Key_0 + (vk - '0');
            }
            else
            {
                if(vk == VK_ESCAPE)
                {
                    key_input = I_Key_Esc;
                }
                else if(vk >= VK_F1 && vk <= VK_F12)
                {
                    key_input = I_Key_F1 + vk - VK_F1;
                }
                else if(vk == VK_OEM_3)
                {
                    key_input = I_Key_GraveAccent;
                }
                else if(vk == VK_OEM_MINUS)
                {
                    key_input = I_Key_Minus;
                }
                else if(vk == VK_OEM_PLUS)
                {
                    key_input = I_Key_Equal;
                }
                else if(vk == VK_BACK)
                {
                    key_input = I_Key_Backspace;
                }
                else if(vk == VK_TAB)
                {
                    key_input = I_Key_Tab;
                }
                else if(vk == VK_SPACE)
                {
                    key_input = I_Key_Space;
                }
                else if(vk == VK_RETURN)
                {
                    key_input = I_Key_Enter;
                }
                else if(vk == VK_CONTROL)
                {
                    key_input = I_Key_Ctrl;
                    modifiers &= ~I_Modifiers_Ctrl;
                }
                else if(vk == VK_SHIFT)
                {
                    key_input = I_Key_Shift;
                    modifiers &= ~I_Modifiers_Shift;
                }
                else if(vk == VK_MENU)
                {
                    key_input = I_Key_Alt;
                    modifiers &= ~I_Modifiers_Alt;
                }
                else if(vk == VK_UP)
                {
                    key_input = I_Key_Up;
                }
                else if(vk == VK_LEFT)
                {
                    key_input = I_Key_Left;
                }
                else if(vk == VK_DOWN)
                {
                    key_input = I_Key_Down;
                }
                else if(vk == VK_RIGHT)
                {
                    key_input = I_Key_Right;
                }
                else if(vk == VK_DELETE)
                {
                    key_input = I_Key_Delete;
                }
                else if(vk == VK_INSERT)
                {
                    key_input = I_Key_Insert;
                }
                else if(vk == VK_PRIOR)
                {
                    key_input = I_Key_PageUp;
                }
                else if(vk == VK_NEXT)
                {
                    key_input = I_Key_PageDown;
                }
                else if(vk == VK_HOME)
                {
                    key_input = I_Key_Home;
                }
                else if(vk == VK_END)
                {
                    key_input = I_Key_End;
                }
                else if(vk == VK_OEM_2)
                {
                    key_input = I_Key_ForwardSlash;
                }
                else if(vk == VK_OEM_PERIOD)
                {
                    key_input = I_Key_Period;
                }
                else if(vk == VK_OEM_COMMA)
                {
                    key_input = I_Key_Comma;
                }
                else if(vk == VK_OEM_7)
                {
                    key_input = I_Key_Quote;
                }
                else if(vk == VK_OEM_4)
                {
                    key_input = I_Key_LeftBracket;
                }
                else if(vk == VK_OEM_6)
                {
                    key_input = I_Key_RightBracket;
                }
                else if(vk == 186)
                {
                    key_input = I_Key_Colon;
                }
            }
            
            EV_Data event =
            {
                .kind = EV_Kind_Key,
                .modifiers = modifiers,
                .key = key_input,
                .is_down = is_down,
            };
            EV_QueuePush(&w->events, event);
            
            w->is_key_down[key_input] = is_down;
            
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
        
        case(WM_CHAR):
        {
            unsigned int char_input = w_param;
            if(VK_RETURN == char_input ||
               (char_input >= 32 &&
                char_input != VK_ESCAPE &&
                char_input != 127))
            {
                if(VK_RETURN == char_input)
                {
                    char_input = '\n';
                }
                EV_Data event =
                {
                    .kind = EV_Kind_Char,
                    .modifiers = modifiers,
                    .codepoint = char_input,
                };
                EV_QueuePush(&w->events, event);
            }
        } break;
        
        default:
        {
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
    }
    
    return result;
}

Function void
W32_WindowDraw(W32_Window *w)
{
    if(0 != w->draw && 0 != w->render_target_view)
    {
        M_ArenaClear(&w->frame_arena);
        R_CmdQueueRecordingBegin(&w->r_cmd_queue);
        w->draw(w);
        R_CmdQueueRecordingEnd(&w->r_cmd_queue);
        R_CmdQueueSort(&w->r_cmd_queue);
        R_CmdQueueExec(&w->r_cmd_queue, w);
        ID3D11DeviceContext_ResolveSubresource(w32_g_app.device_ctx,
                                               w->backbuffer_render_target_texture, 0,
                                               w->render_target_texture, 0,
                                               DXGI_FORMAT_R8G8B8A8_UNORM);
    }
}

Function Bool
W_Update(W_Handle window)
{
    W32_Window *w = window;
    
    w->frame_start = w->frame_end;
    w->frame_end = T_SecondsGet();
    
    EV_QueueClear(&w->events);
    
    MSG message;
    while(!w->should_close && PeekMessageW(&message, w->window_handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    
    W32_WindowDraw(w);
    
    // TODO(tbt): swap buffer on all windows all together at end of main loop
    
    HRESULT hr = IDXGISwapChain_Present(w->swap_chain, w->is_vsync ? 1 : 0, 0);
    Assert_(!FAILED(hr), "Failed to present swapchain. Device lost?");
    if(DXGI_STATUS_OCCLUDED == hr)
    {
        // NOTE(tbt): window is minimised, cannot vsync
        //            instead sleep for a little bit
        Sleep(10);
    }
    
    
    Bool result = !w->should_close;
    return result;
}

Function V2I
W_DimensionsGet(W_Handle window)
{
    W32_Window *w = window;
    
    V2I result;
    RECT client_rect;
    GetClientRect(w->window_handle, &client_rect);
    result.x = client_rect.right - client_rect.left;
    result.y = client_rect.bottom - client_rect.top;
    return result;
}

Function I2F
W_ClientRectGet(W_Handle window)
{
    W32_Window *w = window;
    
    I2F result = {0};
    RECT client_rect;
    GetClientRect(w->window_handle, &client_rect);
    result.max.x = client_rect.right - client_rect.left;
    result.max.y = client_rect.bottom - client_rect.top;
    return result;
}

Function V2F
W_MousePositionGet(W_Handle window)
{
    W32_Window *w = window;
    
    V2F result = {0};
    POINT mouse;
    GetCursorPos(&mouse);
    ScreenToClient(w->window_handle, &mouse);
    result.x = mouse.x;
    result.y = mouse.y;
    return result;
}

Function Bool
W_KeyStateGet(W_Handle window, I_Key key)
{
    W32_Window *w = window;
    Bool result = w->is_key_down[key];
    return result;
}

Function I_Modifiers
W_ModifiersMaskGet(W_Handle window)
{
    I_Modifiers result = {0};
    if(W_KeyStateGet(window, I_Key_Ctrl))
    {
        result |= I_Modifiers_Ctrl;
    }
    if(W_KeyStateGet(window, I_Key_Alt))
    {
        result |= I_Modifiers_Alt;
    }
    if(W_KeyStateGet(window, I_Key_Shift))
    {
        result |= I_Modifiers_Shift;
    }
    return result;
}

Function void
W_FullscreenSet(W_Handle window, Bool is_fullscreen)
{
    W32_Window *w = window;
    
    DWORD window_style = GetWindowLong(w->window_handle, GWL_STYLE);
    
    if(is_fullscreen)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if(GetWindowPlacement(w->window_handle, &w->prev_window_placement) &&
           GetMonitorInfo(MonitorFromWindow(w->window_handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
            SetWindowLong(w->window_handle, GWL_STYLE,
                          window_style & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(w->window_handle, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right -
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom -
                         monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(w->window_handle, GWL_STYLE,
                      window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(w->window_handle, &w->prev_window_placement);
        SetWindowPos(w->window_handle, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

Function void
W_VSyncSet(W_Handle window, Bool is_vsync)
{
    W32_Window *w = window;
    w->is_vsync = is_vsync;
}

Function void
W_CursorKindSet(W_Handle window, W_CursorKind kind)
{
    W32_Window *w = window;
    w->cursor_kind = kind;
}

Function double
W_FrameTimeGet(W_Handle window)
{
    W32_Window *w = window;
    double result = w->frame_end - w->frame_start;
    return result;
}

Function M_Arena *
W_FrameArenaGet(W_Handle window)
{
    W32_Window *w = window;
    M_Arena *result = &w->frame_arena;
    return result;
}

Function EV_Queue *
W_EventQueueGet(W_Handle window)
{
    W32_Window *w = window;
    EV_Queue *result = &w->events;
    return result;
}

Function void
W_UserDataSet(W_Handle window, void *data)
{
    W32_Window *w = window;
    w->user_data = data;
}

Function void *
W_UserDataGet(W_Handle window)
{
    W32_Window *w = window;
    void *result = w->user_data;
    return result;
}
