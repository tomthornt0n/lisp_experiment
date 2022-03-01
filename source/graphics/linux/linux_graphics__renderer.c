
//~NOTE(tbt): resources

Function LINUX_Sprite *
LINUX_SpriteAllocate(void)
{
    SemaphoreWait(linux_sprite_allocator.lock);
    {
        if(0 == linux_sprite_allocator.arena.base)
        {
            linux_sprite_allocator.arena = M_ArenaMake(m_default_hooks);
            linux_sprite_allocator.free_list = &linux_sprite_nil;
        }
        LINUX_Sprite *result = linux_sprite_allocator.free_list;
        linux_sprite_allocator.free_list = linux_sprite_allocator.free_list->free_list_next;
        if(R_SpriteIsNil((R_Sprite *)result))
        {
            result = M_ArenaPush(&linux_sprite_allocator.arena, sizeof(*result));
        }
        result->free_list_next = &linux_sprite_nil;
    }
    SemaphoreSignal(linux_sprite_allocator.lock);
    
    return result;
}

Function void
LINUX_SpriteFree(LINUX_Sprite *sprite)
{
    SemaphoreWait(linux_sprite_allocator.lock);
    sprite->free_list_next = linux_sprite_allocator.free_list;
    linux_sprite_allocator.free_list = sprite;
    SemaphoreSignal(linux_sprite_allocator.lock);
}

Function R_Sprite *
R_SpriteNil(void)
{
    R_Sprite *result = (R_Sprite *)&linux_sprite_nil;
    return result;
}

Function R_Sprite *
R_SpriteMake(Pixel *data, V2I dimensions)
{
    LINUX_Sprite *result = LINUX_SpriteAllocate();
    
    result->parent.dimensions = dimensions;
    
    
    
    return (R_Sprite *)result;
}

Function void
R_SpriteDestroy(R_Sprite *sprite)
{
    if(!R_SpriteIsNil(sprite))
    {
        LINUX_Sprite *s = (LINUX_Sprite *)sprite;
        ID3D11ShaderResourceView_Release(s->texture_view);
        if(0 != s->texture)
        {
            ID3D11Texture2D_Release(s->texture);
        }
        M_Set(s, 0, sizeof(*s));
        LINUX_SpriteFree(s);
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
    result->glyph_cache_atlas = (R_Sprite *)LINUX_SpriteAllocate();
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
    LINUX_Sprite *glyph_cache_sprite = (LINUX_Sprite *)result->glyph_cache_atlas;
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

//~NOTE(tbt): 

Function void
R_CmdQueueExec(R_CmdQueue *q, W_Handle window)
{
    
}
