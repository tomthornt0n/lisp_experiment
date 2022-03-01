
//~NOTE(tbt): resources

Function W32_Sprite *
W32_SpriteAllocate(void)
{
    SemaphoreWait(w32_sprite_allocator.lock);
    
    if(0 == w32_sprite_allocator.arena.base)
    {
        w32_sprite_allocator.arena = M_ArenaMake(m_default_hooks);
        w32_sprite_allocator.free_list = &w32_sprite_nil;
    }
    W32_Sprite *result = w32_sprite_allocator.free_list;
    w32_sprite_allocator.free_list = w32_sprite_allocator.free_list->free_list_next;
    if(R_SpriteIsNil((R_Sprite *)result))
    {
        result = M_ArenaPush(&w32_sprite_allocator.arena, sizeof(*result));
    }
    result->free_list_next = &w32_sprite_nil;
    
    SemaphoreSignal(w32_sprite_allocator.lock);
    return result;
}

Function void
W32_SpriteFree(W32_Sprite *sprite)
{
    SemaphoreWait(w32_sprite_allocator.lock);
    sprite->free_list_next = w32_sprite_allocator.free_list;
    w32_sprite_allocator.free_list = sprite;
    SemaphoreSignal(w32_sprite_allocator.lock);
}

Function R_Sprite *
R_SpriteNil(void)
{
    R_Sprite *result = (R_Sprite *)&w32_sprite_nil;
    return result;
}

Function R_Sprite *
R_SpriteMake(Pixel *data, V2I dimensions)
{
    W32_Sprite *result = W32_SpriteAllocate();
    
    result->parent.dimensions = dimensions;
    
    D3D11_TEXTURE2D_DESC desc =
    {
        .Width = dimensions.x,
        .Height = dimensions.y,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = { 1, 0, },
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };
    D3D11_SUBRESOURCE_DATA subresource_data =
    {
        .pSysMem = (unsigned int *)data,
        .SysMemPitch = dimensions.x*sizeof(unsigned int),
    };
    
    ID3D11Texture2D *texture;
    ID3D11Device_CreateTexture2D(w32_g_app.device,
                                 &desc, &subresource_data,
                                 &texture);
    ID3D11Device_CreateShaderResourceView(w32_g_app.device,
                                          (ID3D11Resource*)texture, 0,
                                          &result->texture_view);
    ID3D11Texture2D_Release(texture);
    
    Assert(0 != result->texture_view);
    
    return (R_Sprite *)result;
}

Function void
R_SpriteDestroy(R_Sprite *sprite)
{
    if(!R_SpriteIsNil(sprite))
    {
        W32_Sprite *s = (W32_Sprite *)sprite;
        ID3D11ShaderResourceView_Release(s->texture_view);
        if(0 != s->texture)
        {
            ID3D11Texture2D_Release(s->texture);
        }
        M_Set(s, 0, sizeof(*s));
        W32_SpriteFree(s);
    }
}

Function R_Font *
R_FontMake(S8 font_data, int size)
{
    //-NOTE(tbt): setup memory
    M_Temp scratch = TC_ScratchGet(0, 0);
    M_Arena arena = M_ArenaMake(m_default_hooks);
#if Build_NoCRT
    if(0 == r_arena_for_stb_truetype.base)
    {
        r_arena_for_stb_truetype = M_ArenaMake(m_default_hooks);
    }
#endif
    
    //-NOTE(tbt): allocate font struct
    R_Font *result = M_ArenaPush(&arena, sizeof(*result));
    result->arena = arena;
    result->glyph_cache_atlas = (R_Sprite *)W32_SpriteAllocate();
    result->font_file = S8Clone(&arena, font_data);
    
    //-NOTE(tbt): get font info
    int ascent, descent, line_gap;
    stbtt_InitFont(&result->font_info, result->font_file.buffer, 0);
    stbtt_GetFontVMetrics(&result->font_info, &ascent, &descent, &line_gap);
    result->scale = stbtt_ScaleForPixelHeight(&result->font_info, size);
    result->v_advance = size + result->scale*line_gap;
    result->is_kerning_enabled = stbtt_GetKerningTableLength(&result->font_info);
    
    //-NOTE(tbt): pack ASCII atlas
    // TODO(tbt): completely breaks with large font sizes as not all ASCII characters fit in atlas
    //            allocate a larger atlas for big fonts
    stbtt_pack_context spc;
    unsigned char *pixels = M_ArenaPush(scratch.arena, R_Font_AsciiAtlasDimensions*R_Font_AsciiAtlasDimensions);
    stbtt_PackBegin(&spc, pixels,
                    R_Font_AsciiAtlasDimensions,
                    R_Font_AsciiAtlasDimensions,
                    0, 2, 0);
    stbtt_PackSetOversampling(&spc, 2, 2);
    Bool succesfully_packed_ascii_atlas = stbtt_PackFontRange(&spc, result->font_file.buffer, 0, size,
                                                              R_Font_AsciiAtlasPackRangeBegin,
                                                              R_Font_AsciiAtlasPackRangeEnd - R_Font_AsciiAtlasPackRangeBegin,
                                                              result->ascii_atlas_info);
    Assert(succesfully_packed_ascii_atlas);
    stbtt_PackEnd(&spc);
    Pixel *pixels_32_bit = M_ArenaPush(scratch.arena, R_Font_AsciiAtlasDimensions*R_Font_AsciiAtlasDimensions*4);
    for(size_t i = 0;
        i < R_Font_AsciiAtlasDimensions*R_Font_AsciiAtlasDimensions;
        i += 1)
    {
        pixels_32_bit[i].r = pixels[i];
        pixels_32_bit[i].g = pixels[i];
        pixels_32_bit[i].b = pixels[i];
        pixels_32_bit[i].a = pixels[i];
    }
    result->ascii_atlas = R_SpriteMake(pixels_32_bit, U2I(R_Font_AsciiAtlasDimensions));
    
    //-NOTE(tbt): setup glyph cache
    W32_Sprite *glyph_cache_sprite = (W32_Sprite *)result->glyph_cache_atlas;
    D3D11_TEXTURE2D_DESC glyph_cache_texture_desc =
    {
        .Width = R_Font_GlyphCacheDimensionsInPixels,
        .Height = R_Font_GlyphCacheDimensionsInPixels,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = { 1, 0 },
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    
    ID3D11Device_CreateTexture2D(w32_g_app.device,
                                 &glyph_cache_texture_desc, 0,
                                 &glyph_cache_sprite->texture);
    ID3D11Device_CreateShaderResourceView(w32_g_app.device,
                                          (ID3D11Resource *)glyph_cache_sprite->texture,
                                          0,
                                          &glyph_cache_sprite->texture_view);
    glyph_cache_sprite->parent.dimensions = U2I(R_Font_GlyphCacheDimensionsInPixels);
    for(size_t cache_cell_index = 0;
        cache_cell_index < R_Font_GlyphCacheCellsCount;
        cache_cell_index += 1)
    {
        result->glyph_cache_info[cache_cell_index].gi = R_Font_GlyphCacheCellUnoccupied;
    }
    
    M_TempEnd(&scratch);
    return result;
}

//~NOTE(tbt): batches

Function void
W32_RenderBatchQuadPush(W32_RenderBatch *batch,
                        I2F rect,
                        I2F uv,
                        V4F colour)
{
    if(SAT2F(batch->mask, rect) && colour.a > 0.0f)
    {
        size_t base = batch->vertices_count;
        
        batch->vertices[batch->vertices_count++] = (R_Vertex)
        {
            .position = { rect.min.x, rect.min.y, },
            .uv = { uv.min.x, uv.min.y, },
            .colour = colour,
        };
        batch->vertices[batch->vertices_count++] = (R_Vertex)
        {
            .position = { rect.min.x, rect.max.y, },
            .uv = { uv.min.x, uv.max.y, },
            .colour = colour,
        };
        batch->vertices[batch->vertices_count++] = (R_Vertex)
        {
            .position = { rect.max.x, rect.max.y, },
            .uv = { uv.max.x, uv.max.y, },
            .colour = colour,
        };
        batch->vertices[batch->vertices_count++] = (R_Vertex)
        {
            .position = { rect.max.x, rect.min.y, },
            .uv = { uv.max.x, uv.min.y, },
            .colour = colour,
        };
        
        batch->indices[batch->indices_count++] = base + 0;
        batch->indices[batch->indices_count++] = base + 1;
        batch->indices[batch->indices_count++] = base + 2;
        batch->indices[batch->indices_count++] = base + 0;
        batch->indices[batch->indices_count++] = base + 2;
        batch->indices[batch->indices_count++] = base + 3;
        
        batch->should_flush = True;
    }
}

Function void
W32_RenderBatchArcPush(W32_RenderBatch *batch,
                       V2F pos,
                       float r,
                       I1F segment,
                       V4F colour)
{
    if(colour.a > 0.0f)
    {
        size_t base = batch->vertices_count;
        
        float theta = (segment.max - segment.min) / (float)(R_ArcVertices - 1);
        float tan_factor = Tan1F(theta);
        float rad_factor = Cos1F(theta);
        
        float x = r*Cos1F(segment.min);
        float y = r*Sin1F(segment.min);
        
        batch->vertices[batch->vertices_count++] = (R_Vertex){ .position = { pos.x, pos.y, }, .colour = colour };
        for(int i = 0;
            i < R_ArcVertices;
            i += 1)
        {
            batch->vertices[batch->vertices_count++] = (R_Vertex){ .position = { pos.x + x, pos.y + y, }, .colour = colour };
            float tx = -y;
            float ty = +x;
            x += tx*tan_factor;
            y += ty*tan_factor;
            x *= rad_factor;
            y *= rad_factor;
            
            batch->indices[batch->indices_count++] = base + 0;
            batch->indices[batch->indices_count++] = base + i;
            batch->indices[batch->indices_count++] = base + i + 1;
        }
        
        batch->should_flush = True;
    }
}

Function void
W32_RenderBatchFlush(W32_RenderBatch *batch)
{
    if(batch->should_flush)
    {
        //-NOTE(tbt): upload uniforms
        D3D11_MAPPED_SUBRESOURCE uniform_buffer_mapped;
        ID3D11DeviceContext_Map(w32_g_app.device_ctx, (ID3D11Resource*)w32_r_data.uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &uniform_buffer_mapped);
        M_Copy(uniform_buffer_mapped.pData, &w32_r_data.uniforms, sizeof(w32_r_data.uniforms));
        ID3D11DeviceContext_Unmap(w32_g_app.device_ctx, (ID3D11Resource*)w32_r_data.uniform_buffer, 0);
        
        //-NOTE(tbt): upload vertices
        D3D11_MAPPED_SUBRESOURCE vertex_buffer_mapped;
        ID3D11DeviceContext_Map(w32_g_app.device_ctx, (ID3D11Resource*)w32_r_data.vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_buffer_mapped);
        M_Copy(vertex_buffer_mapped.pData, batch->vertices, batch->vertices_count*sizeof(batch->vertices[0]));
        ID3D11DeviceContext_Unmap(w32_g_app.device_ctx, (ID3D11Resource*)w32_r_data.vertex_buffer, 0);
        
        //-NOTE(tbt): upload indices
        D3D11_MAPPED_SUBRESOURCE index_buffer_mapped;
        ID3D11DeviceContext_Map(w32_g_app.device_ctx, (ID3D11Resource*)w32_r_data.index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &index_buffer_mapped);
        M_Copy(index_buffer_mapped.pData, batch->indices, batch->indices_count*sizeof(batch->indices[0]));
        ID3D11DeviceContext_Unmap(w32_g_app.device_ctx, (ID3D11Resource*)w32_r_data.index_buffer, 0);
        
        //-NOTE(tbt): input assembler
        ID3D11DeviceContext_IASetInputLayout(w32_g_app.device_ctx, batch->shader->input_layout);
        ID3D11DeviceContext_IASetPrimitiveTopology(w32_g_app.device_ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11DeviceContext_IASetVertexBuffers(w32_g_app.device_ctx, 0, 1, &w32_r_data.vertex_buffer, (UINT[1]){ sizeof(R_Vertex) }, (UINT[1]){ 0 });
        ID3D11DeviceContext_IASetIndexBuffer(w32_g_app.device_ctx, w32_r_data.index_buffer, DXGI_FORMAT_R16_UINT, 0);
        
        //-NOTE(tbt): vertex shader
        ID3D11DeviceContext_VSSetConstantBuffers(w32_g_app.device_ctx, 0, 1, &w32_r_data.uniform_buffer);
        ID3D11DeviceContext_VSSetShader(w32_g_app.device_ctx, batch->shader->vertex_shader, 0, 0);
        
        //-NOTE(tbt): rasteriser
        ID3D11DeviceContext_RSSetState(w32_g_app.device_ctx, w32_r_data.rasteriser_state);
        RECT scissor_rect =
        {
            .left = batch->mask.min.x,
            .top = batch->mask.min.y,
            .right = batch->mask.max.x,
            .bottom = batch->mask.max.y,
        };
        ID3D11DeviceContext_RSSetScissorRects(w32_g_app.device_ctx, 1, &scissor_rect);
        
        //-NOTE(tbt): pixel shader
        ID3D11DeviceContext_PSSetSamplers(w32_g_app.device_ctx, 0, 1, &w32_r_data.sampler);
        ID3D11DeviceContext_PSSetShaderResources(w32_g_app.device_ctx, 0, 1, &batch->texture->texture_view);
        ID3D11DeviceContext_PSSetShader(w32_g_app.device_ctx, batch->shader->pixel_shader, 0, 0);
        
        //-NOTE(tbt): output merger
        ID3D11DeviceContext_OMSetBlendState(w32_g_app.device_ctx, w32_r_data.blend_state, 0, ~0U);
        ID3D11DeviceContext_OMSetDepthStencilState(w32_g_app.device_ctx, w32_r_data.depth_stencil_state, 0);
        ID3D11DeviceContext_OMSetRenderTargets(w32_g_app.device_ctx, 1, &batch->window->render_target_view, batch->window->depth_stencil_view);
        
        //-NOTE(tbt): draw
        ID3D11DeviceContext_DrawIndexed(w32_g_app.device_ctx, batch->indices_count, 0, 0);
    }
    
    batch->vertices_count = 0;
    batch->indices_count = 0;
    batch->texture = 0;
    batch->shader = 0;
    batch->should_flush = False;
}

//~NOTE(tbt): queue processing (renderer backend implementation)

Function void
R_CmdQueueExec(R_CmdQueue *q, W_Handle window)
{
    W32_Window *w = window;
    
    Persist W32_RenderBatch *batch = 0;
    if(0 == batch)
    {
        batch = M_ArenaPush(&TC_Get()->permanent_arena, sizeof(*batch));
    }
    batch->window = w;
    
    w32_r_data.uniforms.u_view_projection = w->projection_matrix;
    D3D11_VIEWPORT viewport =
    {
        .TopLeftX = 0,
        .TopLeftY = 0,
        .Width = w->dimensions.x,
        .Height = w->dimensions.y,
        .MinDepth = 0,
        .MaxDepth = 1,
    };
    ID3D11DeviceContext_RSSetViewports(w32_g_app.device_ctx, 1, &viewport);
    
    for(size_t cmd_index = 0;
        cmd_index < q->cmds_count;
        cmd_index += 1)
    {
        R_Cmd *cmd = &q->cmds[cmd_index];
        
        if(0 == cmd->mask.max.x - cmd->mask.min.x &&
           0 == cmd->mask.max.y - cmd->mask.min.y)
        {
            cmd->mask = W_ClientRectGet(window);
        }
        
        switch(cmd->kind)
        {
            default: break;
            
            case(R_CmdKind_Clear):
            {
                if(0 != w->backbuffer_render_target_view &&
                   0 != w->depth_stencil_view &&
                   0 != w->render_target_view)
                {
                    W32_RenderBatchFlush(batch);
                    ID3D11DeviceContext_ClearRenderTargetView(w32_g_app.device_ctx, w->backbuffer_render_target_view, cmd->colour.elements);
                    ID3D11DeviceContext_ClearRenderTargetView(w32_g_app.device_ctx, w->render_target_view, cmd->colour.elements);
                    ID3D11DeviceContext_ClearDepthStencilView(w32_g_app.device_ctx, w->depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
                }
            } break;
            
            case(R_CmdKind_DrawSprite):
            case_R_CmdKind_DrawSprite:;
            {
                if(!Eq(batch->mask, cmd->mask) ||
                   batch->shader != &w32_r_data.default_shader ||
                   batch->texture != (W32_Sprite *)cmd->sprite ||
                   batch->indices_count > R_Batch_MaxIndices - 6 ||
                   batch->vertices_count > R_Batch_MaxVertices - 4)
                {
                    W32_RenderBatchFlush(batch);
                    batch->shader = &w32_r_data.default_shader;
                    batch->texture = (W32_Sprite *)cmd->sprite;
                    batch->mask = cmd->mask;
                }
                W32_RenderBatchQuadPush(batch, cmd->rect, cmd->sub_sprite, cmd->colour);
            } break;
            
            case(R_CmdKind_DrawRectStroke):
            {
                if(!Eq(batch->mask, cmd->mask) ||
                   batch->shader != &w32_r_data.default_shader ||
                   batch->texture != (W32_Sprite *)cmd->sprite ||
                   batch->indices_count > R_Batch_MaxIndices - 24 ||
                   batch->vertices_count > R_Batch_MaxVertices - 8)
                {
                    W32_RenderBatchFlush(batch);
                    batch->shader = &w32_r_data.default_shader;
                    batch->texture = (W32_Sprite *)cmd->sprite;
                    batch->mask = cmd->mask;
                }
                
                size_t base = batch->vertices_count;
                
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.min.x, cmd->rect.min.y, },
                    .uv = { cmd->sub_sprite.min.x, cmd->sub_sprite.min.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.min.x, cmd->rect.max.y, },
                    .uv = { cmd->sub_sprite.min.x, cmd->sub_sprite.max.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.max.x, cmd->rect.max.y, },
                    .uv = { cmd->sub_sprite.max.x, cmd->sub_sprite.max.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.max.x, cmd->rect.min.y, },
                    .uv = { cmd->sub_sprite.max.x, cmd->sub_sprite.min.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.min.x + cmd->size, cmd->rect.min.y + cmd->size, },
                    .uv = { cmd->sub_sprite.min.x, cmd->sub_sprite.min.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.min.x + cmd->size, cmd->rect.max.y - cmd->size, },
                    .uv = { cmd->sub_sprite.min.x, cmd->sub_sprite.max.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.max.x - cmd->size, cmd->rect.max.y - cmd->size, },
                    .uv = { cmd->sub_sprite.max.x, cmd->sub_sprite.max.y, },
                    .colour = cmd->colour,
                };
                batch->vertices[batch->vertices_count++] = (R_Vertex)
                {
                    .position = { cmd->rect.max.x - cmd->size, cmd->rect.min.y + cmd->size, },
                    .uv = { cmd->sub_sprite.max.x, cmd->sub_sprite.min.y, },
                    .colour = cmd->colour,
                };
                
                batch->indices[batch->indices_count++] = base + 0;
                batch->indices[batch->indices_count++] = base + 1;
                batch->indices[batch->indices_count++] = base + 5;
                batch->indices[batch->indices_count++] = base + 0;
                batch->indices[batch->indices_count++] = base + 5;
                batch->indices[batch->indices_count++] = base + 4;
                
                batch->indices[batch->indices_count++] = base + 5;
                batch->indices[batch->indices_count++] = base + 1;
                batch->indices[batch->indices_count++] = base + 2;
                batch->indices[batch->indices_count++] = base + 5;
                batch->indices[batch->indices_count++] = base + 2;
                batch->indices[batch->indices_count++] = base + 6;
                
                batch->indices[batch->indices_count++] = base + 7;
                batch->indices[batch->indices_count++] = base + 6;
                batch->indices[batch->indices_count++] = base + 2;
                batch->indices[batch->indices_count++] = base + 7;
                batch->indices[batch->indices_count++] = base + 2;
                batch->indices[batch->indices_count++] = base + 3;
                
                batch->indices[batch->indices_count++] = base + 0;
                batch->indices[batch->indices_count++] = base + 4;
                batch->indices[batch->indices_count++] = base + 7;
                batch->indices[batch->indices_count++] = base + 0;
                batch->indices[batch->indices_count++] = base + 7;
                batch->indices[batch->indices_count++] = base + 3;
                
                batch->should_flush = True;
            } break;
            
            case(R_CmdKind_DrawS8):
            {
                R_Font *font = cmd->font;
                V2F pos = cmd->rect.min;
                
                int i = 0;
                for(UTFConsume consume = CodepointFromUTF8(cmd->string, i);
                    i < cmd->string.len;
                    (i += consume.advance), (consume = CodepointFromUTF8(cmd->string, i)))
                {
                    unsigned int c = consume.codepoint;
                    int gi = stbtt_FindGlyphIndex(&font->font_info, c);
                    
                    int kern = 0;
                    if(cmd->font->is_kerning_enabled)
                    {
                        int j = i + consume.advance;
                        UTFConsume next_consume = CodepointFromUTF8(cmd->string, j);
                        if(j + next_consume.advance < cmd->string.len)
                        {
                            int next_gi = stbtt_FindGlyphIndex(&font->font_info, next_consume.codepoint);
                            kern = stbtt_GetGlyphKernAdvance(&font->font_info, gi, next_gi);
                        }
                    }
                    
                    if('\n' == c)
                    {
                        pos.x = cmd->rect.min.x;
                        pos.y += font->v_advance;
                    }
                    else if(CharIsSpace(c))
                    {
                        int advance, left_side_bearing;
                        stbtt_GetGlyphHMetrics(&font->font_info, gi, &advance, &left_side_bearing);
                        pos.x += (advance + kern)*font->scale;
                    }
                    else if(R_Font_AsciiAtlasPackRangeBegin <= c && c < R_Font_AsciiAtlasPackRangeEnd)
                    {
                        if(!Eq(batch->mask, cmd->mask) ||
                           batch->shader != &w32_r_data.default_shader ||
                           batch->texture != (W32_Sprite *)font->ascii_atlas ||
                           batch->indices_count > R_Batch_MaxIndices - 6 ||
                           batch->vertices_count > R_Batch_MaxVertices - 4)
                        {
                            W32_RenderBatchFlush(batch);
                            batch->shader = &w32_r_data.default_shader;
                            batch->texture = (W32_Sprite *)font->ascii_atlas;
                            batch->mask = cmd->mask;
                        }
                        stbtt_aligned_quad q;
                        stbtt_GetPackedQuad(font->ascii_atlas_info,
                                            R_Font_AsciiAtlasDimensions,
                                            R_Font_AsciiAtlasDimensions,
                                            c - R_Font_AsciiAtlasPackRangeBegin,
                                            &pos.x, &pos.y, &q, False);
                        
                        pos.x += kern*font->scale;
                        I2F rect = I2F(V2F(q.x0, q.y0), V2F(q.x1, q.y1));
                        I2F uv = I2F(V2F(q.s0, q.t0), V2F(q.s1, q.t1));
                        if(rect.min.y - font->v_advance > batch->mask.max.y)
                        {
                            break;
                        }
                        
                        W32_RenderBatchQuadPush(batch, rect, uv, cmd->colour);
                    }
                    else
                    {
#if Build_NoCRT
                        M_Temp stb_truetype_arena_checkpoint = M_TempBegin(&r_arena_for_stb_truetype);
#endif
                        
                        unsigned int cell_index = Abs(Noise1U(gi)) % R_Font_GlyphCacheCellsCount;
                        for(int probe_index = cell_index;
                            probe_index < R_Font_GlyphCacheCellsCount;
                            probe_index += 1)
                        {
                            R_FontGlyphCacheCellInfo *cell_info = &font->glyph_cache_info[probe_index];
                            if(gi == cell_info->gi || R_Font_GlyphCacheCellUnoccupied == cell_info->gi)
                            {
                                cell_index = probe_index;
                                break;
                            }
                        }
                        R_FontGlyphCacheCellInfo *cell_info = &font->glyph_cache_info[cell_index];
                        if(gi != cell_info->gi)
                        {
                            W32_RenderBatchFlush(batch);
                            batch->shader = &w32_r_data.default_shader;
                            batch->texture = (W32_Sprite *)font->glyph_cache_atlas;
                            
                            
                            V2F cell_origin = V2F((cell_index % R_Font_GlyphCacheGridSize)*R_Font_GlyphCacheCellSize,
                                                  (cell_index / R_Font_GlyphCacheGridSize)*R_Font_GlyphCacheCellSize);
                            
                            int advance;
                            stbtt_GetGlyphHMetrics(&cmd->font->font_info, gi, &advance, 0);
                            advance += kern;
                            advance *= cmd->font->scale;
                            
                            int ix_0;
                            int ix_1;
                            int iy_0;
                            int iy_1;
                            stbtt_GetGlyphBitmapBox(&cmd->font->font_info, gi,
                                                    cmd->font->scale,
                                                    cmd->font->scale,
                                                    &ix_0, &iy_0, &ix_1, &iy_1);
                            // NOTE(tbt): will break with large font sizes as glyph does not fit in cell
                            // TODO(tbt): fallback to mega stupid thing if glyph too big to fit in cache cell
                            
                            cell_info->gi = gi;
                            cell_info->dimensions = V2F(ix_1 - ix_0, iy_1 - iy_0);
                            cell_info->bearing = V2I(ix_0, iy_0);
                            cell_info->advance = advance;
                            cell_info->uv = R_SubSpriteFromSprite(font->glyph_cache_atlas, RectMake2F(cell_origin, cell_info->dimensions));
                            
                            unsigned char glyph_pixels[R_Font_GlyphCacheBytesPerCell];
                            stbtt_MakeGlyphBitmap(&cmd->font->font_info,
                                                  glyph_pixels,
                                                  R_Font_GlyphCacheCellSize,
                                                  R_Font_GlyphCacheCellSize,
                                                  R_Font_GlyphCacheCellSize,
                                                  cmd->font->scale,
                                                  cmd->font->scale,
                                                  gi);
                            
                            for(size_t y = 0;
                                y < R_Font_GlyphCacheCellSize;
                                y += 1)
                            {
                                for(size_t x = 0;
                                    x < R_Font_GlyphCacheCellSize;
                                    x += 1)
                                {
                                    int dst_x = x + cell_origin.x;
                                    int dst_y = y + cell_origin.y;
                                    
                                    int dst_index = (dst_y*R_Font_GlyphCacheDimensionsInPixels + dst_x)*4;
                                    int src_index = y*R_Font_GlyphCacheCellSize + x;
                                    
                                    font->glyph_cache_pixels[dst_index + 0] = glyph_pixels[src_index];
                                    font->glyph_cache_pixels[dst_index + 1] = glyph_pixels[src_index];
                                    font->glyph_cache_pixels[dst_index + 2] = glyph_pixels[src_index];
                                    font->glyph_cache_pixels[dst_index + 3] = glyph_pixels[src_index];
                                }
                            }
                            
                            D3D11_MAPPED_SUBRESOURCE texture_mapped;
                            ID3D11Texture2D *glyph_cache_texture = ((W32_Sprite *)font->glyph_cache_atlas)->texture;
                            ID3D11DeviceContext_Map(w32_g_app.device_ctx, (ID3D11Resource *)glyph_cache_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &texture_mapped);
                            M_Copy(texture_mapped.pData, font->glyph_cache_pixels, R_Font_GlyphCacheSizeInBytes);
                            ID3D11DeviceContext_Unmap(w32_g_app.device_ctx, (ID3D11Resource *)glyph_cache_texture, 0);
                        }
                        
                        if(!Eq(batch->mask, cmd->mask) ||
                           batch->shader != &w32_r_data.default_shader ||
                           batch->texture != (W32_Sprite *)font->glyph_cache_atlas ||
                           batch->indices_count > R_Batch_MaxIndices - 6 ||
                           batch->vertices_count > R_Batch_MaxVertices - 4)
                        {
                            W32_RenderBatchFlush(batch);
                            batch->shader = &w32_r_data.default_shader;
                            batch->texture = (W32_Sprite *)font->glyph_cache_atlas;
                            batch->mask = cmd->mask;
                        }
                        
                        I2F rect = RectMake2F(V2F(pos.x + cell_info->bearing.x,
                                                  pos.y + cell_info->bearing.y),
                                              cell_info->dimensions);
                        if(rect.min.y - cmd->font->v_advance > batch->mask.max.y)
                        {
                            break;
                        }
                        
                        W32_RenderBatchQuadPush(batch, rect, cell_info->uv, cmd->colour);
                        pos.x += cell_info->advance;
                        
#if Build_NoCRT
                        M_TempEnd(&stb_truetype_arena_checkpoint);
#endif
                    }
                }
            } break;
            
            case(R_CmdKind_DrawCircleFill):
            {
                if(!Eq(batch->mask, cmd->mask) ||
                   batch->shader != &w32_r_data.default_shader ||
                   batch->texture != &w32_sprite_nil ||
                   batch->indices_count > R_Batch_MaxIndices - 3*R_ArcVertices ||
                   batch->vertices_count > R_Batch_MaxVertices - R_ArcVertices)
                {
                    W32_RenderBatchFlush(batch);
                    batch->shader = &w32_r_data.default_shader;
                    batch->texture = &w32_sprite_nil;
                    batch->mask = cmd->mask;
                }
                W32_RenderBatchArcPush(batch, cmd->rect.min, cmd->size, I1F(0.0f, 2.0f*Pi), cmd->colour);
            } break;
            
            case(R_CmdKind_DrawRoundedRect):
            {
                cmd->size = Min1F(cmd->size, 0.5f*Min1F(Dimensions2F(cmd->rect).x, Dimensions2F(cmd->rect).y));
                if(cmd->size < 2.0f)
                {
                    goto case_R_CmdKind_DrawSprite;
                }
                
                size_t required_v = 4*R_ArcVertices + 5*4;
                size_t required_i = 12*R_ArcVertices + 5*6;
                
                if(!Eq(batch->mask, cmd->mask) ||
                   batch->shader != &w32_r_data.default_shader ||
                   batch->texture != &w32_sprite_nil ||
                   batch->indices_count > R_Batch_MaxIndices - required_i ||
                   batch->vertices_count > R_Batch_MaxVertices - required_v)
                {
                    W32_RenderBatchFlush(batch);
                    batch->shader = &w32_r_data.default_shader;
                    batch->texture = &w32_sprite_nil;
                    batch->mask = cmd->mask;
                }
                
                V2F positions[] =
                {
                    [2] = Add2F(cmd->rect.min, U2F(cmd->size)),
                    [0] = Sub2F(cmd->rect.max, U2F(cmd->size)),
                    [3] = V2F(cmd->rect.max.x - cmd->size, cmd->rect.min.y + cmd->size),
                    [1] = V2F(cmd->rect.min.x + cmd->size, cmd->rect.max.y - cmd->size),
                };
                for(int i = 0;
                    i < 4;
                    i += 1)
                {
                    W32_RenderBatchArcPush(batch, positions[i], cmd->size, I1F(i*0.5f*Pi, (i + 1)*0.5f*Pi), cmd->colour);
                }
                W32_RenderBatchQuadPush(batch, I2F(Sub2F(positions[2], V2F(cmd->size, 0.0f)), positions[1]), I2F(0), cmd->colour);
                W32_RenderBatchQuadPush(batch, I2F(Sub2F(positions[2], V2F(0.0f, cmd->size)), positions[3]), I2F(0), cmd->colour);
                W32_RenderBatchQuadPush(batch, I2F(positions[1], Add2F(positions[0], V2F(0.0f, cmd->size))), I2F(0), cmd->colour);
                W32_RenderBatchQuadPush(batch, I2F(positions[3], Add2F(positions[0], V2F(cmd->size, 0.0f))), I2F(0), cmd->colour);
                W32_RenderBatchQuadPush(batch, I2F(positions[2], positions[0]), I2F(0), cmd->colour);
            } break;
            
            case(R_CmdKind_SubQueue):
            {
                R_CmdQueueExec(cmd->sub_queue, window);
            } break;
        }
    }
    
    W32_RenderBatchFlush(batch);
}