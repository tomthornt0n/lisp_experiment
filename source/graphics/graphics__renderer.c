
//~NOTE(tbt): sprites

Function Bool
R_SpriteIsNil(R_Sprite *sprite)
{
    Bool result = (0 == sprite || R_SpriteNil() == sprite);
    return result;
}

Function R_SubSprite
R_SubSpriteFromSprite(R_Sprite *sprite, I2F region)
{
    V2F dimensions_f = V2F(sprite->dimensions.x, sprite->dimensions.y);
    I2F result =
    {
        .min = Div2F(region.min, dimensions_f),
        .max = Div2F(region.max, dimensions_f),
    };
    return result;
}

//~NOTE(tbt): fonts / text

Function R_Font *
R_FontFromFile(S8 filename, int size)
{
    R_Font *result = 0;
    
    M_Temp scratch = TC_ScratchGet(0, 0);
    S8 file = F_ReadEntire(scratch.arena, filename);
    if(0 != file.buffer)
    {
        result = R_FontMake(file, size);
    }
    M_TempEnd(&scratch);
    
    return result;
}

Function void
R_FontDestroy(R_Font *font)
{
    R_SpriteDestroy(font->ascii_atlas);
    R_SpriteDestroy(font->glyph_cache_atlas);
    M_ArenaDestroy(&font->arena);
}

Function R_MeasuredText
R_MeasureS8(M_Arena *arena,
            R_Font *font,
            S8 string,
            V2F position)
{
    // TODO(tbt): this is a complete mess
    
    R_MeasuredText result = {0};
    
    result.position = position;
    result.string = S8Clone(arena, string);
    result.codepoints = S32FromS8(arena, string);
    
    V2F cur = position;
    Bool is_start_of_line = True;
    for(size_t i = 0; i < result.codepoints.len; i += 1)
    {
        unsigned int c = result.codepoints.buffer[i];
        
        int gi = stbtt_FindGlyphIndex(&font->font_info, c);
        
        int kern = 0;
        if(font->is_kerning_enabled)
        {
            if((i + 1) < result.codepoints.len)
            {
                int gi_1 = stbtt_FindGlyphIndex(&font->font_info, result.codepoints.buffer[i + 1]);
                kern = stbtt_GetGlyphKernAdvance(&font->font_info, gi, gi_1);
                kern *= font->scale;
            }
        }
        
        if('\n' == c)
        {
            
            struct R_MeasuredTextRect *r = M_ArenaPushAligned(arena, sizeof(*r), 1);
            r->base_line = cur.y;
            r->bounds.min = V2F(cur.x, cur.y - font->v_advance);
            r->bounds.max = V2F(cur.x + 2.0f, cur.y);
            if(is_start_of_line)
            {
                r->closest.min = V2F(FLT_MIN, cur.y - font->v_advance);
                r->closest.max = V2F(FLT_MAX, cur.y);
            }
            if(0 == result.rects)
            {
                result.rects = r;
            }
            result.rects_count += 1;
            cur.x = position.x;
            cur.y += font->v_advance;
            if(i > 0)
            {
                result.rects[i - 1].closest.max.x = FLT_MAX;
                is_start_of_line = True;
                
                result.bounds.min = Mins2F(result.bounds.min, r->bounds.min);
                result.bounds.max = Maxs2F(result.bounds.max, r->bounds.max);
            }
            else
            {
                result.bounds = r->bounds;
            }
        }
        else if(CharIsSpace(c))
        {
            int advance;
            int left_side_bearing;
            stbtt_GetGlyphHMetrics(&font->font_info, gi, &advance, &left_side_bearing);
            cur.x += (advance + kern)*font->scale;
            
            struct R_MeasuredTextRect *r = M_ArenaPushAligned(arena, sizeof(*r), 1);
            r->base_line = cur.y;
            r->bounds.min = V2F(cur.x - advance*font->scale, cur.y - font->v_advance);
            r->bounds.max = V2F(cur.x, cur.y);
            if(0 == i)
            {
                result.bounds = r->bounds;
            }
            else
            {
                result.bounds.min = Mins2F(result.bounds.min, r->bounds.min);
                result.bounds.max = Maxs2F(result.bounds.max, r->bounds.max);
            }
            r->closest = r->bounds;
            if(is_start_of_line)
            {
                r->closest.min.x = FLT_MIN;
                is_start_of_line = False;
            }
            if(0 == result.rects)
            {
                result.rects = r;
            }
            result.rects_count += 1;
        }
        else if(c >= R_Font_AsciiAtlasPackRangeBegin &&
                c < R_Font_AsciiAtlasPackRangeEnd)
        {
            float x = cur.x;
            
            stbtt_aligned_quad q;
            stbtt_GetPackedQuad(font->ascii_atlas_info,
                                R_Font_AsciiAtlasDimensions,
                                R_Font_AsciiAtlasDimensions,
                                c - R_Font_AsciiAtlasPackRangeBegin,
                                &cur.x, &cur.y,
                                &q, False);
            cur.x += kern*font->scale;
            
            struct R_MeasuredTextRect *r = M_ArenaPushAligned(arena, sizeof(*r), 1);
            r->base_line = cur.y;
            r->bounds = I2F(V2F(q.x0, q.y0), V2F(q.x1, q.y1));
            r->closest = I2F(V2F(x, cur.y - font->v_advance), V2F(cur.x, cur.y));;
            if(0 == i)
            {
                result.bounds = r->bounds;
            }
            else
            {
                result.bounds.min = Mins2F(result.bounds.min, r->bounds.min);
                result.bounds.max = Maxs2F(result.bounds.max, r->bounds.max);
            }
            if(is_start_of_line)
            {
                r->closest.min.x = FLT_MIN;
                is_start_of_line = False;
            }
            if(0 == result.rects)
            {
                result.rects = r;
            }
            result.rects_count += 1;
        }
        else
        {
            int advance;
            stbtt_GetGlyphHMetrics(&font->font_info, gi, &advance, 0);
            advance *= font->scale;
            advance += kern;
            
            int ix_0;
            int ix_1;
            int iy_0;
            int iy_1;
            stbtt_GetGlyphBitmapBox(&font->font_info, gi,
                                    font->scale,
                                    font->scale,
                                    &ix_0, &iy_0, &ix_1, &iy_1);
            
            struct R_MeasuredTextRect *r = M_ArenaPushAligned(arena, sizeof(*r), 1);
            r->base_line = cur.y;
            r->bounds = RectMake2F(V2F(cur.x + ix_0, cur.y + iy_0), V2F(ix_1 - ix_0, iy_1 - iy_0));
            r->closest = I2F(V2F(cur.x, cur.y - font->v_advance), V2F(cur.x + advance, cur.y));
            if(0 == i)
            {
                result.bounds = r->bounds;
            }
            else
            {
                result.bounds.min = Mins2F(result.bounds.min, r->bounds.min);
                result.bounds.max = Maxs2F(result.bounds.max, r->bounds.max);
            }
            if(is_start_of_line)
            {
                r->closest.min.x = FLT_MIN;
                is_start_of_line = False;
            }
            if(0 == result.rects)
            {
                result.rects = r;
            }
            result.rects_count += 1;
            
            cur.x += advance;
        }
    }
    
    result.bounds.max.x = Max1F(result.bounds.max.x, result.bounds.min.x);
    result.bounds.max.y = Max1F(result.bounds.max.y, result.bounds.min.y);
    
    return result;
}

Function R_MeasuredTextIndex
R_MeasuredTextIndexFromPosition(R_MeasuredText measured_text,
                                V2F pixel_position)
{
    R_MeasuredTextIndex result = R_MeasuredTextIndex_None;
    
    for(size_t rect_index = 0;
        rect_index < measured_text.rects_count;
        rect_index += 1)
    {
        if(IntervalHasValue2F(measured_text.rects[rect_index].bounds, pixel_position))
        {
            result = rect_index;
        }
    }
    
    return result;
}

Function R_MeasuredTextIndex
R_MeasuredTextNearestIndexFromPosition(R_MeasuredText measured_text,
                                       V2F pixel_position)
{
    R_MeasuredTextIndex result = (pixel_position.y < measured_text.rects[0].closest.min.y) ? 0 : measured_text.rects_count - 1;
    
    for(size_t rect_index = 0;
        rect_index < measured_text.rects_count;
        rect_index += 1)
    {
        if(IntervalHasValue2F(measured_text.rects[rect_index].closest, pixel_position))
        {
            result = rect_index;
            
            float centre_x = Centre2F(measured_text.rects[rect_index].closest).x;
            if(pixel_position.x > centre_x)
            {
                result += 1;
            }
        }
    }
    
    return result;
}

Function V2F
R_AdvanceFromS8(R_Font *font, S8 string)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    R_MeasuredText mt = R_MeasureS8(scratch.arena, font, string, U2F(0.0f));
    V2F result = Dimensions2F(mt.bounds);
    M_TempEnd(&scratch);
    return result;
}


//~NOTE(tbt): commands

Function void
R_CmdPush(R_CmdQueue *q, R_Cmd cmd)
{
    Assert(0 != q);
    if(q->cmds_count < R_CmdQueue_Cap)
    {
        q->cmds[q->cmds_count] = cmd;
        q->cmds_count += 1;
    }
}

Global unsigned char r_layer_stack[128];
Global size_t r_layer_stack_count = 1;

Function void
R_LayerPush(unsigned char sort_key)
{
    if(r_layer_stack_count < ArrayCount(r_layer_stack))
    {
        r_layer_stack[r_layer_stack_count] = sort_key;
        r_layer_stack_count += 1;
    }
}

Function void
R_LayerPop(void)
{
    if(r_layer_stack_count > 1)
    {
        r_layer_stack_count -= 1;
    }
}

Global I2F r_mask_stack[128];
Global size_t r_mask_stack_count = 1;

Function void
R_MaskPush(I2F mask)
{
    if(r_mask_stack_count < ArrayCount(r_mask_stack))
    {
        r_mask_stack[r_mask_stack_count] = mask;
        r_mask_stack_count += 1;
    }
}

Function void
R_MaskPushIntersecting(I2F mask)
{
    R_MaskPush(Intersection2F(mask, r_mask_stack[r_mask_stack_count - 1]));
}

Function void
R_MaskPop(void)
{
    if(r_mask_stack_count > 1)
    {
        r_mask_stack_count -= 1;
    }
}


Global R_CmdQueue *r_recording_cmd_queue_stack[128];
Global size_t r_recording_cmd_queue_stack_count = 0;

Function void
R_CmdQueueRecordingBegin(R_CmdQueue *q)
{
    if(r_recording_cmd_queue_stack_count < ArrayCount(r_recording_cmd_queue_stack))
    {
        r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count] = q;
        r_recording_cmd_queue_stack_count += 1;
        q->cmds_count = 0;
    }
}

Function void
R_CmdQueueRecordingEnd(R_CmdQueue *q)
{
    R_CmdQueue **top = &r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    Assert(q == *top);
    if(r_recording_cmd_queue_stack_count > 1)
    {
        r_recording_cmd_queue_stack_count -= 1;
        *top = 0;
    }
}

Function void
R_CmdMake_(R_Cmd *cmd)
{
    cmd->sort_key = (r_layer_stack[r_layer_stack_count - 1] % (1 << 16)) << 16;
    cmd->sort_key |= cmd->kind << 8;
    cmd->sort_key |= IntFromPtr(cmd->sprite) % 255;
    cmd->mask = r_mask_stack[r_mask_stack_count - 1];
}

Function void
R_CmdClear(V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_Clear,
        .colour = colour,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}

Function void
R_CmdDrawSprite(R_Sprite *sprite,
                I2F rect,
                V4F colour)
{
    R_CmdDrawSubSprite(sprite,
                       r_entire_texture,
                       rect,
                       colour);
}

Function void
R_CmdDrawSubSprite(R_Sprite *sprite,
                   R_SubSprite sub_sprite,
                   I2F rect,
                   V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_DrawSprite,
        .sprite = sprite,
        .sub_sprite = sub_sprite,
        .rect = rect,
        .colour = colour,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}

Function void
R_CmdDrawRectFill(I2F rect,
                  V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_DrawSprite,
        .sprite = R_SpriteNil(),
        .sub_sprite = r_entire_texture,
        .rect = rect,
        .colour = colour,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}

Function void
R_CmdDrawRectStroke(I2F rect,
                    float stroke_width,
                    V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_DrawRectStroke,
        .sprite = R_SpriteNil(),
        .rect = I2F(Mins2F(rect.min, rect.max), Maxs2F(rect.min, rect.max)),
        .colour = colour,
        .size = stroke_width,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}

Function void
R_CmdDrawS8(R_Font *font,
            S8 string,
            V2F position,
            V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_DrawS8,
        .rect.min = position,
        .sprite = font->ascii_atlas,
        .colour = colour,
        .string = string,
        .font = font,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}

Function void
R_CmdDrawRoundedRect(I2F rect,
                     float radius,
                     V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_DrawRoundedRect,
        .sprite = R_SpriteNil(),
        .rect = rect,
        .colour = colour,
        .size = radius,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}

Function void
R_CmdDrawCircleFill(V2F position,
                    float radius,
                    V4F colour)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_DrawCircleFill,
        .rect = I2F(position),
        .colour = colour,
        .size = radius,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}


Function void
R_CmdSubQueue(R_CmdQueue *sub_queue)
{
    R_Cmd cmd =
    {
        .kind = R_CmdKind_SubQueue,
        .sub_queue = sub_queue,
    };
    R_CmdMake_(&cmd);
    R_CmdQueue *q = r_recording_cmd_queue_stack[r_recording_cmd_queue_stack_count - 1];
    R_CmdPush(q, cmd);
}


Function int
R_CmdQueueSort_Comparator_(const void *a,
                           const void *b,
                           void *user_data)
{
    const R_Cmd *_a = a;
    const R_Cmd *_b = b;
    int difference = _a->sort_key - _b->sort_key;
    return difference;
}

Function void
R_CmdQueueSort(R_CmdQueue *q)
{
    Sort(q->cmds, q->cmds_count, sizeof(q->cmds[0]), R_CmdQueueSort_Comparator_, 0);
}
