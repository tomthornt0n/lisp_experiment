
//~NOTE(tbt): rect-cut layouts

Function I2F
UI_CutLeft(I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->min.x, rect->min.y),
        .max = V2F(rect->min.x + f, rect->max.y),
    };
    rect->min.x += f;
    return result;
}

Function I2F
UI_CutRight(I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->max.x - f, rect->min.y),
        .max = V2F(rect->max.x, rect->max.y),
    };
    rect->max.x -= f;
    return result;
}

Function I2F
UI_CutTop(I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->min.x, rect->min.y),
        .max = V2F(rect->max.x, rect->min.y + f),
    };
    rect->min.y += f;
    return result;
}

Function I2F
UI_CutBottom (I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->min.x, rect->max.y - f),
        .max = V2F(rect->max.x, rect->max.y),
    };
    rect->max.y -= f;
    return result;
}

Function I2F
UI_Cut(I2F *rect, UI_CutDir dir, float f)
{
    I2F result;
    switch(dir)
    {
        case(UI_CutDir_Left):
        {
            result = UI_CutLeft(rect, f);
        } break;
        
        case(UI_CutDir_Right):
        {
            result = UI_CutRight(rect, f);
        } break;
        
        case(UI_CutDir_Top):
        {
            result = UI_CutTop(rect, f);
        } break;
        
        case(UI_CutDir_Bottom):
        {
            result = UI_CutBottom(rect, f);
        } break;
    }
    return result;
}

Function I2F
UI_GetLeft(I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.min.x, rect.min.y),
        .max = V2F(rect.min.x + f, rect.max.y),
    };
    rect.min.x += f;
    return result;
}

Function I2F
UI_GetRight(I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.max.x - f, rect.min.y),
        .max = V2F(rect.max.x, rect.max.y),
    };
    rect.max.x -= f;
    return result;
}

Function I2F
UI_GetTop(I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.min.x, rect.min.y),
        .max = V2F(rect.max.x, rect.min.y + f),
    };
    rect.min.y += f;
    return result;
}

Function I2F
UI_GetBottom (I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.min.x, rect.max.y - f),
        .max = V2F(rect.max.x, rect.max.y),
    };
    rect.max.y -= f;
    return result;
}

Function I2F
UI_Get(I2F rect, UI_CutDir dir, float f)
{
    I2F result;
    switch(dir)
    {
        case(UI_CutDir_Left):
        {
            result = UI_GetLeft(rect, f);
        } break;
        
        case(UI_CutDir_Right):
        {
            result = UI_GetRight(rect, f);
        } break;
        
        case(UI_CutDir_Top):
        {
            result = UI_GetTop(rect, f);
        } break;
        
        case(UI_CutDir_Bottom):
        {
            result = UI_GetBottom(rect, f);
        } break;
    }
    return result;
}


//~NOTE(tbt): scrollable rows layouts

Function void
UI_ScrollableBegin(UI_Scrollable *state,
                   I2F rect,
                   float row_height)
{
    state->target_scroll = Min1F(state->target_scroll, 0.0f);
    state->rect = rect;
    state->height_per_row = row_height;
    float max_scroll = Max1F(state->rows_count*state->height_per_row - (rect.max.y - rect.min.y), 0.0f);
    
    W_Handle window = G_CurrentWindowGet();
    
    if(IntervalHasValue2F(rect, W_MousePositionGet(window)))
    {
        state->target_scroll += 25.0f*EV_QueueScrollGet(W_EventQueueGet(window)).y;
    }
    state->target_scroll = Clamp1F(state->target_scroll, -max_scroll, 0.0f);
    state->scroll = UI_AnimateTowards1F(state->scroll, state->target_scroll, 25.0f);
    state->rows_count = 0;
}

Function I2F
UI_ScrollableRow(UI_Scrollable *state)
{
    float position = state->rect.min.y + state->rows_count*state->height_per_row + Round1F(state->scroll);
    I2F result = I2F(V2F(state->rect.min.x, position),
                     V2F(state->rect.max.x, position + state->height_per_row));
    state->rows_count += 1;
    return result;
}

//~NOTE(tbt): text editing

Function void
UI_EditTextDeleteRange(S8 *s, size_t cap, I1U *selection)
{
    if(selection->cursor != selection->mark && s->len > 0)
    {
        I1U range =
        {
            .min = Min(selection->cursor, selection->mark),
            .max = Max(selection->cursor, selection->mark),
        };
        
        M_Copy(&s->buffer[range.min],
               &s->buffer[range.max],
               cap - range.max);
        if(s->len > range.max - range.min)
        {
            s->len -= range.max - range.min;
        }
        else
        {
            s->len = 0;
        }
        
        selection->cursor = range.min;
        selection->mark = range.min;
    }
}

Function Bool
UI_EditText(char buffer[], size_t cap,
            I1U *selection,
            size_t *len,
            Bool should_consume,
            UI_EditTextFilterHook filter_hook, void *filter_hook_user_data)
{
    cap -= 1;
    
    S8 s = (S8){ .buffer = buffer, };
    if(0 == len)
    {
        s.len = CStringCalculateUTF8Length(buffer);
    }
    else
    {
        s.len = *len;
    }
    
    char character_insertion_buffer[5] = {0};
    
    enum
    {
        ActionFlags_MoveCursor = Bit(0),
        ActionFlags_PreDelete  = Bit(1),
        ActionFlags_PostDelete = Bit(2),
        ActionFlags_Insert     = Bit(3),
        ActionFlags_Copy       = Bit(4),
        ActionFlags_SelectAll  = Bit(5),
        
        ActionFlags_StickMark  = Bit(6),
        ActionFlags_WordLevel  = Bit(7),
        
        ActionFlags_EDIT = (ActionFlags_PreDelete |
                            ActionFlags_PostDelete |
                            ActionFlags_Insert),
    };
    struct
    {
        unsigned char flags;
        int offset;
        S8 to_insert;
    } action = {0};
    
    M_Temp scratch = TC_ScratchGet(0, 0);
    EV_Queue *events = W_EventQueueGet(G_CurrentWindowGet());
    
    for(EV_QueueForEach(events, e))
    {
        M_Set(&action, 0, sizeof(action));
        
        Bool consume = False;
        
        if(EV_Kind_Key == e->kind && e->is_down)
        {
            if((selection->cursor != selection->mark && !(e->modifiers & I_Modifiers_Shift)) &&
               (I_Key_Left == e->key || I_Key_Right == e->key ||
                I_Key_Home == e->key || I_Key_End == e->key))
            {
                selection->mark = (I_Key_Left == e->key || I_Key_Home == e->key) ? Min1U(selection->min, selection->max) : Max1U(selection->min, selection->max);
                selection->cursor = selection->mark;
                G_ForceNextUpdate();
                consume = True;
            }
            else if(I_Key_Left == e->key || I_Key_Right == e->key)
            {
                action.flags |= ActionFlags_MoveCursor;
                action.offset = (I_Key_Left == e->key) ? -1 : +1;
                G_ForceNextUpdate();
                consume = True;
            }
            else if(I_Key_Home == e->key || I_Key_End == e->key)
            {
                action.flags |= ActionFlags_MoveCursor;
                action.offset = (I_Key_Home == e->key) ? INT_MIN : INT_MAX;
                G_ForceNextUpdate();
                consume = True;
            }
            else if(I_Key_Backspace == e->key || I_Key_Delete == e->key)
            {
                action.flags |= ActionFlags_PostDelete;
                if(selection->mark == selection->cursor)
                {
                    action.flags |= ActionFlags_MoveCursor;
                    action.flags |= ActionFlags_StickMark;
                    action.offset = (I_Key_Delete == e->key) ? +1 : -1;
                }
                G_ForceNextUpdate();
                consume = True;
            }
            
            if(0 != (e->modifiers & I_Modifiers_Ctrl))
            {
                if(I_Key_C == e->key || I_Key_Insert == e->key)
                {
                    action.flags |= ActionFlags_Copy;
                    consume = True;
                }
                else if(I_Key_V == e->key)
                {
                    action.flags |= ActionFlags_Insert;
                    action.flags |= ActionFlags_MoveCursor;
                    action.flags |= ActionFlags_PreDelete;
                    action.flags &= ~ActionFlags_StickMark;
                    action.to_insert = CLIP_TextGet(scratch.arena);
                    action.offset = S8CharIndexFromByteIndex(action.to_insert, action.to_insert.len);
                    G_ForceNextUpdate();
                    consume = True;
                }
                else if(I_Key_X == e->key)
                {
                    action.flags |= ActionFlags_Copy;
                    action.flags |= ActionFlags_PostDelete;
                    G_ForceNextUpdate();
                    consume = True;
                }
                else if(I_Key_A == e->key)
                {
                    action.flags = ActionFlags_SelectAll;
                    G_ForceNextUpdate();
                    consume = True;
                }
                else
                {
                    action.flags |= ActionFlags_WordLevel;
                }
            }
            if(0 != (e->modifiers & I_Modifiers_Shift))
            {
                if(I_Key_Insert == e->key)
                {
                    action.flags |= ActionFlags_Insert;
                    action.flags |= ActionFlags_MoveCursor;
                    action.flags |= ActionFlags_PreDelete;
                    action.flags &= ~ActionFlags_StickMark;
                    action.to_insert = CLIP_TextGet(scratch.arena);
                    action.offset = S8CharIndexFromByteIndex(action.to_insert, action.to_insert.len);
                    G_ForceNextUpdate();
                    consume = True;
                }
                else if(I_Key_Delete == e->key)
                {
                    action.flags |= ActionFlags_Copy;
                    action.flags |= ActionFlags_PostDelete;
                    G_ForceNextUpdate();
                    consume = True;
                }
                else
                {
                    action.flags |= ActionFlags_StickMark;
                }
            }
            
            if(e->key < I_Key_PRINTABLE_END)
            {
                G_ForceNextUpdate();
                consume = True;
            }
        }
        else if(EV_Kind_Char == e->kind)
        {
            // TODO(tbt): multiline text edits
            if('\n' != e->codepoint)
            {
                M_Arena str = M_ArenaFromArray(character_insertion_buffer);
                action.flags |= ActionFlags_Insert;
                action.flags |= ActionFlags_MoveCursor;
                action.flags |= ActionFlags_PreDelete;
                action.to_insert = UTF8FromCodepoint(&str, e->codepoint);
                action.offset = 1;
                G_ForceNextUpdate();
            }
            consume = True;
        }
        
        if(action.flags & ActionFlags_SelectAll)
        {
            selection->mark = 0;
            selection->cursor = s.len;
        }
        
        if(action.flags & ActionFlags_Copy)
        {
            I1U range =
            {
                .min = Min1U(selection->cursor, selection->mark),
                .max = Max1U(selection->cursor, selection->mark),
            };
            S8 selected_string =
            {
                .buffer = &buffer[range.min],
                .len = range.max - range.min
            };
            CLIP_TextSet(selected_string);
        }
        
        if(action.flags & ActionFlags_PreDelete)
        {
            UI_EditTextDeleteRange(&s, cap, selection);
        }
        
        if(action.flags & ActionFlags_Insert)
        {
            if(0 == filter_hook || filter_hook(action.to_insert, filter_hook_user_data))
            {
                char *temp = M_ArenaPush(scratch.arena, cap);
                
                M_Copy(temp, buffer, selection->cursor);
                M_Copy(&temp[selection->cursor + action.to_insert.len],
                       &buffer[selection->cursor],
                       cap - selection->cursor - action.to_insert.len);
                M_Copy(&temp[selection->cursor],
                       action.to_insert.buffer,
                       action.to_insert.len);
                M_Copy(buffer, temp, cap);
                
                s.len += action.to_insert.len;
            }
        }
        
        if(action.flags & ActionFlags_MoveCursor)
        {
            int dir = Normalise1I(action.offset);
            while(action.offset)
            {
                for(;;)
                {
                    selection->cursor += dir;
                    if(selection->cursor > s.len)
                    {
                        selection->cursor -= dir;
                        break;
                    }
                    else if(!(UTF8IsContinuationByte(s, selection->cursor) ||
                              (action.flags & ActionFlags_WordLevel) &&
                              !S8IsWordBoundary(s, selection->cursor)))
                    {
                        break;
                    }
                }
                action.offset -= dir;
                
                if(selection->cursor == 0 ||
                   selection->cursor >= s.len)
                {
                    break;
                }
            }
            selection->cursor = Clamp1U(selection->cursor, 0, s.len);
            if(!(action.flags & ActionFlags_StickMark))
            {
                selection->mark = selection->cursor;
            }
        }
        
        if(action.flags & ActionFlags_PostDelete)
        {
            UI_EditTextDeleteRange(&s, cap, selection);
        }
        
        
        if(should_consume && consume)
        {
            EV_Consume(e);
        }
    }
    
    if(0 != len)
    {
        *len = s.len;
    }
    else
    {
        s.buffer[s.len] = '\0';
    }
    
    M_TempEnd(&scratch);
    Bool is_edited = !!(action.flags & ActionFlags_EDIT);
    return is_edited;
}

Function void
UI_DrawS8WithCaret(R_Font *font,
                   R_MeasuredText mt,
                   V4F colour,
                   I1U *selection)
{
    R_CmdDrawS8(font, S8Clone(W_FrameArenaGet(G_CurrentWindowGet()), mt.string), mt.position, colour);
    
    if(mt.string.len > 0)
    {
        M_Temp scratch = TC_ScratchGet(0, 0);
        W_Handle window = G_CurrentWindowGet();
        EV_Queue *events = W_EventQueueGet(window);
        
        if(IntervalHasValue2F(Expand2F(mt.bounds, U2F(4.0f)), W_MousePositionGet(window)))
        {
            for(EV_QueueForEach(events, e))
            {
                if(EV_Kind_Key == e->kind &&
                   I_Key_MouseButtonLeft == e->key && e->is_down)
                {
                    R_MeasuredTextIndex clicked_char_index = R_MeasuredTextNearestIndexFromPosition(mt, W_MousePositionGet(window));
                    if(R_MeasuredTextIndex_None != clicked_char_index)
                    {
                        size_t clicked_byte_index = S8ByteIndexFromCharIndex(mt.string, clicked_char_index);
                        selection->cursor = clicked_byte_index;
                        selection->mark = clicked_byte_index;
                        EV_Consume(e);
                    }
                }
                else if(EV_Kind_MouseMove == e->kind && W_KeyStateGet(window, I_Key_MouseButtonLeft))
                {
                    R_MeasuredTextIndex clicked_char_index = R_MeasuredTextNearestIndexFromPosition(mt, W_MousePositionGet(window));
                    if(R_MeasuredTextIndex_None != clicked_char_index)
                    {
                        size_t clicked_byte_index = S8ByteIndexFromCharIndex(mt.string, clicked_char_index);
                        selection->cursor = clicked_byte_index;
                        EV_Consume(e);
                    }
                }
            }
        }
        
        I1U char_indices =
        {
            .cursor = S8CharIndexFromByteIndex(mt.string, selection->cursor),
            .mark = S8CharIndexFromByteIndex(mt.string, selection->mark),
        };
        
        V2F cursor_origin = mt.position;
        V2F mark_origin = mt.position;
        if(char_indices.cursor > 0)
        {
            cursor_origin = V2F(mt.rects[char_indices.cursor - 1].bounds.max.x,
                                mt.rects[char_indices.cursor - 1].base_line);
        }
        if(char_indices.mark > 0)
        {
            mark_origin = V2F(mt.rects[char_indices.mark - 1].bounds.max.x,
                              mt.rects[char_indices.mark - 1].base_line);
        }
        
        if(selection->mark != selection->cursor)
        {
            I2F highlight =
            {
                .min =
                {
                    .x = Min1F(cursor_origin.x, mark_origin.x),
                    .y = cursor_origin.y - font->v_advance,
                },
                .max =
                {
                    .x = Max1F(cursor_origin.x, mark_origin.x),
                    .y = cursor_origin.y,
                },
            };
            R_LayerRelative(+1) R_CmdDrawRectFill(highlight, Scale4F(colour, 0.5f));
        }
        
        I2F caret = RectMake2F(cursor_origin, V2F(2.0f, -font->v_advance));
        R_LayerRelative(+1) R_CmdDrawRectFill(caret, colour);
        
        M_TempEnd(&scratch);
    }
    else
    {
        I2F caret = RectMake2F(mt.position, V2F(2.0f, -font->v_advance));
        R_LayerRelative(+1) R_CmdDrawRectFill(caret, colour);
    }
}

//~NOTE(tbt): misc.

Function V2F
UI_TextPositionFromRect(R_Font *font, I2F rect, V2F padding)
{
    V2F result = Add2F(rect.min, V2F(padding.x, 0.5f*(Dimensions2F(rect).y + font->v_advance) - padding.y));
    return result;
}

Function void
UI_DrawLineShadow(Bool flip, I2F rect, float intensity)
{
    Persist R_Sprite *sprite = 0;
    if(0 == sprite)
    {
        Persist Pixel pixels[] =
        {
            { .r = 0, .g = 0, .b = 0, .a = (uint8_t)(255.0*0.198596102131253140), },
            { .r = 0, .g = 0, .b = 0, .a = (uint8_t)(255.0*0.175713634395793070), },
            { .r = 0, .g = 0, .b = 0, .a = (uint8_t)(255.0*0.121702746509626260), },
            { .r = 0, .g = 0, .b = 0, .a = (uint8_t)(255.0*0.065983967749849120), },
            { .r = 0, .g = 0, .b = 0, .a = (uint8_t)(255.0*0.028001560233780885), },
            { .r = 0, .g = 0, .b = 0, .a = (uint8_t)(255.0*0.009300040045324049), },
        };
        sprite = R_SpriteMake(pixels, V2I(1, 6));
    }
    
    V2F dimensions = Dimensions2F(rect);
    R_SubSprite uv;
    if(flip)
    {
        uv = R_SubSpriteFromSprite(sprite, I2F(V2F(0.0f, 6.0f), V2F(dimensions.x, 0.0f)));
    }
    else
    {
        uv = R_SubSpriteFromSprite(sprite, I2F(V2F(0.0f, 0.0f), V2F(dimensions.x, 6.0f)));
    }
    R_CmdDrawSubSprite(sprite, uv, rect, U4F(intensity));
}

//~NOTE(tbt): animation helpers


Function void
UI_AnimateInterpolateF_(float result[], size_t n,
                        float a[], float b[],
                        float t)
{
    for(size_t i = 0; i < n; i += 1)
    {
        result[i] = InterpolateLinear1F(a[i], b[i], t);
        if(UI_AnimateSlop < t && t < 1.0f - UI_AnimateSlop)
        {
            G_ForceNextUpdate();
        }
    }
}

Function Bool
UI_AnimateLinearF(float result[], size_t n,
                  float a[], float b[],
                  double start, double duration)
{
    float t = Clamp1F((T_SecondsGet() - start) / duration, 0.0f, 1.0f);
    UI_AnimateInterpolateF_(result, n, a, b, t);
    return (1.0f == t);
}

Function Bool
UI_AnimateExponentialF(float result[], size_t n,
                       float a[], float b[],
                       double start, double duration)
{
    float t = Clamp1F((T_SecondsGet() - start) / duration, 0.0f, 1.0f);
    t *= t;
    UI_AnimateInterpolateF_(result, n, a, b, t);
    return (1.0f == t);
}

Function Bool
UI_AnimateSmoothF(float result[], size_t n,
                  float a[], float b[],
                  double start, double duration)
{
    float t = Smoothstep1F(Clamp1F((T_SecondsGet() - start) / duration, 0.0f, 1.0f));
    UI_AnimateInterpolateF_(result, n, a, b, t);
    return (1.0f == t);
}

Function float
UI_AnimateInOut1F(float a, float b,
                  double start_time,
                  double in_time,
                  double sustain,
                  double out_time)
{
    double t = T_SecondsGet() - start_time;
    float i = Clamp1F(t / in_time, 0.0f, 1.0f);
    float o = 1.0f - Clamp1F((t - in_time - sustain) / out_time, 0.0f, 1.0f);
    float result = InterpolateLinear1F(a, b, i*o);
    if(t < (in_time + sustain + out_time))
    {
        G_ForceNextUpdate();
    }
    return result;
}

Function float
UI_AnimateTowards1F(float a, float b, float speed)
{
    float result = b;
    if(Abs1F(a - b) > UI_AnimateSlop)
    {
        double t = Min1F(speed*W_FrameTimeGet(G_CurrentWindowGet()), 0.5f);
        result = InterpolateLinear1F(a, b, t);
        G_ForceNextUpdate();
    }
    return result;
}

Function void
UI_AnimateTowardsF(float a[], float b[], size_t n, float speed)
{
    for(size_t i = 0;
        i < n;
        i += 1)
    {
        a[i] = UI_AnimateTowards1F(a[i], b[i], speed);
    }
}
