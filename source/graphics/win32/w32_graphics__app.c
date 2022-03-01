
Function void
G_Init(void)
{
    //-NOTE(tbt): initialise OS layer
    OS_Init();
    
    //-NOTE(tbt): get instance handle (not passed in to wWinMain if building without CRT)
    w32_g_app.instance_handle = GetModuleHandle(0);
    
    //-NOTE(tbt): register the class for windows to use
    WNDCLASSW window_class = 
    {
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = W32_WindowProc,
        .hInstance = w32_g_app.instance_handle,
        .lpszClassName = W32_WindowClassName,
        .hCursor = LoadCursor(0, IDC_ARROW),
    };
    RegisterClassW(&window_class);
    
    //-NOTE(tbt): setup D3D11 device
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if Build_ModeDebug
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D11CreateDevice(0,
                      D3D_DRIVER_TYPE_HARDWARE,
                      0,
                      flags,
                      levels,
                      ArrayCount(levels),
                      D3D11_SDK_VERSION,
                      &w32_g_app.device,
                      0,
                      &w32_g_app.device_ctx);
    
    //-NOTE(tbt): D3D11 debugging
#if Build_ModeDebug
    ID3D11InfoQueue *info;
    IProvideClassInfo_QueryInterface(w32_g_app.device, &IID_ID3D11InfoQueue, (void**)&info);
    ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    ID3D11InfoQueue_Release(info);
#endif
    
    //-NOTE(tbt): nil sprite
    w32_sprite_allocator.lock = SemaphoreMake(1);
    Pixel nil_sprite_data = { 0xffffffff };
    D3D11_TEXTURE2D_DESC nil_sprite_texture_desc =
    {
        .Width = w32_sprite_nil.parent.dimensions.x,
        .Height = w32_sprite_nil.parent.dimensions.y,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = { 1, 0 },
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };
    D3D11_SUBRESOURCE_DATA nil_sprite_subresource_data =
    {
        .pSysMem = (unsigned int *)&nil_sprite_data,
        .SysMemPitch = sizeof(unsigned int),
    };
    ID3D11Texture2D *nil_sprite_texture;
    ID3D11Device_CreateTexture2D(w32_g_app.device,
                                 &nil_sprite_texture_desc,
                                 &nil_sprite_subresource_data,
                                 &nil_sprite_texture);
    ID3D11Device_CreateShaderResourceView(w32_g_app.device,
                                          (ID3D11Resource*)nil_sprite_texture, 0,
                                          &w32_sprite_nil.texture_view);
    ID3D11Texture2D_Release(nil_sprite_texture);
    
    //-NOTE(tbt): default shader
#include "..\build\w32__vshader.h"
#include "..\build\w32__pshader.h"
    D3D11_INPUT_ELEMENT_DESC input_layout_desc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, OffsetOf(R_Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, OffsetOf(R_Vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOUR",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, OffsetOf(R_Vertex, colour),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ID3D11Device_CreateVertexShader(w32_g_app.device,
                                    d3d11_vshader,
                                    sizeof(d3d11_vshader),
                                    0, &w32_r_data.default_shader.vertex_shader);
    ID3D11Device_CreatePixelShader(w32_g_app.device,
                                   d3d11_pshader, sizeof(d3d11_pshader),
                                   0, &w32_r_data.default_shader.pixel_shader);
    ID3D11Device_CreateInputLayout(w32_g_app.device,
                                   input_layout_desc,
                                   ArrayCount(input_layout_desc),
                                   d3d11_vshader, sizeof(d3d11_vshader),
                                   &w32_r_data.default_shader.input_layout);
    
    //-NOTE(tbt): uniform buffer
    D3D11_BUFFER_DESC uniform_buffer_desc =
    {
        .ByteWidth = sizeof(W32_UniformBuffer),
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    ID3D11Device_CreateBuffer(w32_g_app.device,
                              &uniform_buffer_desc,
                              0,
                              &w32_r_data.uniform_buffer);
    
    //-NOTE(tbt): vertex buffer
    D3D11_BUFFER_DESC vertex_buffer_desc =
    {
        .ByteWidth = sizeof(R_Vertex)*R_Batch_MaxVertices,
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    ID3D11Device_CreateBuffer(w32_g_app.device,
                              &vertex_buffer_desc,
                              0,
                              &w32_r_data.vertex_buffer);
    
    //-NOTE(tbt): index buffer
    D3D11_BUFFER_DESC index_buffer_desc =
    {
        .ByteWidth = sizeof(uint16_t)*R_Batch_MaxIndices,
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_INDEX_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    ID3D11Device_CreateBuffer(w32_g_app.device,
                              &index_buffer_desc,
                              0,
                              &w32_r_data.index_buffer);
    
    //-NOTE(tbt): texture sampler
    D3D11_SAMPLER_DESC sampler_desc =
    {
        .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    };
    ID3D11Device_CreateSamplerState(w32_g_app.device,
                                    &sampler_desc,
                                    &w32_r_data.sampler);
    
    //-NOTE(tbt): enable alpha blending
    D3D11_BLEND_DESC blend_state_desc =
    {
        .RenderTarget[0] =
        {
            .BlendEnable = TRUE,
            .SrcBlend = D3D11_BLEND_ONE,
            .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
            .BlendOp = D3D11_BLEND_OP_ADD,
            .SrcBlendAlpha = D3D11_BLEND_ONE,
            .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
            .BlendOpAlpha = D3D11_BLEND_OP_ADD,
            .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
        },
    };
    ID3D11Device_CreateBlendState(w32_g_app.device,
                                  &blend_state_desc,
                                  &w32_r_data.blend_state);
    
    //-NOTE(tbt): disable culling
    D3D11_RASTERIZER_DESC rasteriser_state_desc =
    {
        .FillMode = D3D11_FILL_SOLID,
        .CullMode = D3D11_CULL_NONE,
        .ScissorEnable = TRUE,
        .MultisampleEnable = TRUE,
        .AntialiasedLineEnable = TRUE,
    };
    ID3D11Device_CreateRasterizerState(w32_g_app.device,
                                       &rasteriser_state_desc,
                                       &w32_r_data.rasteriser_state);
    
    //-NOTE(tbt): disable depth and stencil test
    D3D11_DEPTH_STENCIL_DESC depth_stencil_state_desc =
    {
        .DepthEnable = FALSE,
        .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D11_COMPARISON_LESS,
        .StencilEnable = FALSE,
        .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
        .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    };
    ID3D11Device_CreateDepthStencilState(w32_g_app.device,
                                         &depth_stencil_state_desc,
                                         &w32_r_data.depth_stencil_state);
}

Function W_Handle
G_WindowOpen(S8 title,
             V2I dimensions,
             G_AppHooks hooks)
{
    
    //-NOTE(tbt): allocate and make window
    M_Arena arena = M_ArenaMake(m_default_hooks);
    W32_WindowNode *result = M_ArenaPush(&arena, sizeof(*result));
    result->arena = arena;
    result->cleanup = hooks.close;
    hooks.open(result);
    W32_WindowMake(&result->w, title, dimensions, hooks.draw);
    
    //-NOTE(tbt): append to app windows list
    if(0 == w32_g_app.windows_last)
    {
        Assert(0 == w32_g_app.windows_first);
        w32_g_app.windows_first = result;
        w32_g_app.windows_last = result;
    }
    else
    {
        result->prev = w32_g_app.windows_last;
        w32_g_app.windows_last->next = result;
        w32_g_app.windows_last = result;
    }
    
    return result;
}

Function void
G_WindowClose(W_Handle window)
{
    W32_WindowNode *w = window;
    w->w.should_close = True;
}

Function void
W32_WindowNodeRemove(W32_WindowNode *w)
{
    //-NOTE(tbt): remove from windows list
    if(0 == w->prev)
    {
        w32_g_app.windows_first = w->next;
    }
    else
    {
        w->prev->next = w->next;
    }
    if(0 == w->next)
    {
        w32_g_app.windows_last = w->prev;
    }
    else
    {
        w->next->prev = w->prev;
    }
    
    //-NOTE(tbt): destroy window
    W32_WindowDestroy(&w->w);
    
    //-NOTE(tbt): free memory
    M_ArenaDestroy(&w->arena);
}

Function W_Handle
G_WindowsFirstGet(void)
{
    W_Handle result = w32_g_app.windows_first;
    return result;
}

Function W_Handle
G_WindowsNextGet(W_Handle current)
{
    W32_WindowNode *result = current;
    result = result->next;
    return result;
}

Function void
G_MainLoop(void)
{
    while(0 != w32_g_app.windows_first)
    {
        if(w32_g_app.should_block_to_wait_for_events && !w32_g_app.should_force_next_update)
        {
            WaitMessage();
        }
        ITL_Exchange((int volatile *)&w32_g_app.should_force_next_update, False);
        
        W32_WindowNode *next = 0;
        for(W32_WindowNode *w = w32_g_app.windows_first;
            0 != w;
            w = next)
        {
            next = w->next;
            w32_g_app.current_window = w;
            if(!W_Update(w))
            {
                w->cleanup(w);
                W32_WindowNodeRemove(w);
            }
        }
    }
}

Function void
G_Cleanup(void)
{
    W32_WindowNode *next = 0;
    for(W32_WindowNode *w = w32_g_app.windows_first;
        0 != w;
        w = next)
    {
        w->cleanup(w);
        W32_WindowNodeRemove(w);
    }
    OS_Cleanup();
}

Function Bool
G_ShouldBlockToWaitForEventsGet(void)
{
    Bool result = w32_g_app.should_block_to_wait_for_events;
    return result;
}

Function void
G_ShouldBlockToWaitForEventsSet(Bool should_block_to_wait_for_events)
{
    w32_g_app.should_block_to_wait_for_events = should_block_to_wait_for_events;
}

Function void
G_ForceNextUpdate(void)
{
    if(!ITL_CompareExchange((int volatile *)&w32_g_app.should_force_next_update, True, False))
    {
        for(W32_WindowNode *w = w32_g_app.windows_first; 0 != w; w = w->next)
        {
            RedrawWindow(w->w.window_handle, 0, 0, 0);
        }
    }
}

Function W_Handle
G_CurrentWindowGet(void)
{
    W_Handle result = w32_g_app.current_window;
    return result;
}

Function M_Arena *
G_ArenaFromWindow(W_Handle window)
{
    W32_WindowNode *w = window;
    M_Arena *result = &w->arena;
    return result;
}
