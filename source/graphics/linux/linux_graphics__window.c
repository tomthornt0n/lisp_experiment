
Function void
LINUX_EventQueueUpdate(LINUX_EventQueue *queue)
{
    free(queue->prev);
    queue->prev = queue->current;
    queue->current = queue->next;
    if(linux_g_app.should_block_to_wait_for_events && !linux_g_app.should_force_next_update)
    {
        queue->next = xcb_wait_for_event(linux_g_app.connection);
    }
    else
    {
        queue->next = xcb_poll_for_queued_event(linux_g_app.connection);
    }
}

Function void
LINUX_WindowMake(LINUX_Window *window, S8 title, V2I dimensions, W_DrawHook draw)
{
    window->frame_arena = M_ArenaMake(m_default_hooks);
    window->events = EV_QueueMake();
    
    unsigned int value_mask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
    
    window->window = xcb_generate_id(linux_g_app.connection);
    xcb_create_window(linux_g_app.connection,
                      XCB_COPY_FROM_PARENT,
                      window->window,
                      linux_g_app.screen->root,
                      0, 0, dimensions.x, dimensions.y,
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      linux_g_app.visual_id,
                      value_mask, linux_g_app.value_list);
    
    xcb_atom_t window_close_atom = linux_g_app.window_close_reply->atom;
    xcb_change_property(linux_g_app.connection,
                        XCB_PROP_MODE_REPLACE,
                        window->window,
                        linux_g_app.wm_protocol_reply->atom,
                        XCB_ATOM_ATOM,
                        32,
                        1,
                        &window_close_atom);
    
    xcb_change_property(linux_g_app.connection,
                        XCB_PROP_MODE_REPLACE,
                        window->window,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        title.len, title.buffer);
    
    xcb_map_window(linux_g_app.connection, window->window);
    window->drawable = glX.CreateWindow(linux_g_app.display, linux_g_app.framebuffer_config, window->window, 0);
    
    if(0 == window->window)
    {
        xcb_destroy_window(linux_g_app.connection, window->window);
    }
}

Function void
LINUX_WindowDestroy(LINUX_Window *window)
{
    glX.DestroyWindow(linux_g_app.display, window->drawable);
    xcb_destroy_window(linux_g_app.connection, window->window);
    M_ArenaDestroy(&window->frame_arena);
}

Function void
LINUX_WindowDraw(LINUX_Window *window)
{
    if(0 != window->draw)
    {
        M_ArenaClear(&window->frame_arena);
        R_CmdQueueRecord(&window->r_cmd_queue)
        {
            window->draw(window);
        }
        R_CmdQueueSort(&window->r_cmd_queue);
        R_CmdQueueExec(&window->r_cmd_queue, window);
    }
}

Function Bool
W_Update(W_Handle window)
{
    LINUX_Window *w = window;
    
    w->frame_start = w->frame_end;
    w->frame_end = T_SecondsGet();
    
    EV_QueueClear(&w->events);
    
    LINUX_EventQueueUpdate(&w->event_queue);
    while(0 != w->event_queue.current)
    {
        switch(w->event_queue.current->response_type & ~0x80)
        {
            default: break;
            
            case(XCB_KEY_PRESS):
            {
                xcb_key_press_event_t *event = (xcb_key_press_event_t *)w->event_queue.current;
                ConsoleOutputFmt("pressed: %d\n", event->detail);
            };
        }
        LINUX_EventQueueUpdate(&w->event_queue);
    }
    
    glX.MakeContextCurrent(linux_g_app.display, w->drawable,w->drawable, linux_g_app.context);
    LINUX_WindowDraw(window);
    
    Bool result = !w->should_close;
    return result;
}

Function V2I
W_DimensionsGet(W_Handle window)
{
    V2I result = {0};
    
    LINUX_Window *w = window;
    
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(linux_g_app.connection, w->window);
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(linux_g_app.connection, cookie, 0);
    if (0 != reply)
    {
        result.x = reply->width;
        result.y = reply->height;
    }
    free(reply);
    
    return result;
}

Function I2F
W_ClientRectGet(W_Handle window)
{
    V2I dimensions = W_DimensionsGet(window);
    I2F result = I2F(V2F(0.0f, 0.0f), V2F(dimensions.x, dimensions.y));
    return result;
}

Function V2F
W_MousePositionGet(W_Handle window)
{
    V2F result = {0};
    
    LINUX_Window *w = window;
    
    xcb_query_pointer_cookie_t cookie = xcb_query_pointer(linux_g_app.connection, w->window);
    xcb_query_pointer_reply_t *reply = xcb_query_pointer_reply(linux_g_app.connection, cookie, 0);
    if (0 != reply)
    {
        result.x = reply->win_x;
        result.y = reply->win_y;
    }
    free(reply);
    
    return result;
}

Function Bool
W_KeyStateGet(W_Handle window, I_Key key)
{
    LINUX_Window *w = window;
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
    LINUX_Window *w = window;
    
    xcb_intern_atom_reply_t *wm_state_reply = LINUX_XAtomReplyGet("_NET_WM_STATE");
    xcb_intern_atom_reply_t *fullscreen_reply = LINUX_XAtomReplyGet("_NET_WM_STATE_FULLSCREEN");
    
    if (is_fullscreen)                           
    {                                         
        xcb_change_property(linux_g_app.connection,    
                            XCB_PROP_MODE_REPLACE,
                            w->window,
                            wm_state_reply->atom,
                            XCB_ATOM_ATOM,
                            32,
                            1,
                            &fullscreen_reply->atom);
    }
    else 
    {
        xcb_delete_property(linux_g_app.connection, w->window, wm_state_reply->atom);
    }   
    
    free(fullscreen_reply);
    free(wm_state_reply);
    
    xcb_unmap_window(linux_g_app.connection, w->window);  
    xcb_map_window(linux_g_app.connection, w->window);
}

Function void
W_VSyncSet(W_Handle window, Bool is_vsync)
{
    LINUX_Window *w = window;
    if(LINUX_GLHasExtension(linux_g_app.default_screen_id, "GLX_EXT_swap_control"))
    {
        glX.SwapIntervalEXT(linux_g_app.display, w->drawable, is_vsync);
    }
    else if(LINUX_GLHasExtension(linux_g_app.default_screen_id, "GLX_SGI_swap_control"))
    {
        glX.SwapIntervalSGI(is_vsync);
    }
    else if(LINUX_GLHasExtension(linux_g_app.default_screen_id, "GLX_MESA_swap_control"))
    {
        glX.SwapIntervalMESA(is_vsync);
    }
}

Function void
W_CursorKindSet(W_Handle window, W_CursorKind kind)
{
    LINUX_Window *w = window;
    
    xcb_font_t font = xcb_generate_id(linux_g_app.connection);
    xcb_open_font(linux_g_app.connection, font, strlen("cursor"), "cursor");
    
    int id = 2;
    if(kind < W_CursorKind_MAX)
    {
        W_CursorKind lut[W_CursorKind_MAX] =
        {
            [W_CursorKind_Default] = 2,
            [W_CursorKind_HResize] = 108,
            [W_CursorKind_VResize] = 116,
            [W_CursorKind_Hidden] = 0,
        };
        id = lut[kind];
    }
    
    xcb_cursor_t cursor = xcb_generate_id(linux_g_app.connection);
    xcb_create_glyph_cursor(linux_g_app.connection,
                            cursor,
                            font, font,
                            id, id + 1,
                            0, 0, 0, 0, 0, 0);
    
    int mask = XCB_CW_CURSOR;
    uint32_t value_list = cursor;
    xcb_change_window_attributes(linux_g_app.connection, w->window, mask, &value_list);
    
    xcb_free_cursor(linux_g_app.connection, cursor);
    xcb_close_font(linux_g_app.connection, font);
}


Function double
W_FrameTimeGet(W_Handle window)
{
    LINUX_Window *w = window;
    double result = w->frame_end - w->frame_start;
    return result;
}

Function M_Arena *
W_FrameArenaGet(W_Handle window)
{
    LINUX_Window *w = window;
    M_Arena *result = &w->frame_arena;
    return result;
}

Function EV_Queue *
W_EventQueueGet(W_Handle window)
{
    LINUX_Window *w = window;
    EV_Queue *result = &w->events;
    return result;
}

Function void
W_UserDataSet(W_Handle window, void *data)
{
    LINUX_Window *w = window;
    w->user_data = data;
}

Function void *
W_UserDataGet(W_Handle window)
{
    LINUX_Window *w = window;
    void *result = w->user_data;
    return result;
}
