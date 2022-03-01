
//~NOTE(tbt): event data

Function void
EV_Consume (EV_Data *event)
{
    event->is_consumed = True;
}

//~NOTE(tbt): event queues

Function EV_Queue
EV_QueueMake(void)
{
    EV_Queue result = {0};
    result.arena = M_ArenaMake(m_default_hooks);
    result.events = result.arena.base;
    return result;
}

Function void
EV_QueueClear(EV_Queue *queue)
{
    queue->events_count = 0;
    M_ArenaClear(&queue->arena);
}

Function void
EV_QueueDestroy(EV_Queue *queue)
{
    M_ArenaDestroy(&queue->arena);
}

Function void
EV_QueuePush(EV_Queue *queue, EV_Data event)
{
    queue->events_count += 1;
    EV_Data *data = M_ArenaPushAligned(&queue->arena, sizeof(EV_Data), 1);
    M_Copy(data, &event, sizeof(*data));
}

Function EV_Data *
EV_QueueIncrementNotConsumed_(EV_Queue *queue, EV_Data *current)
{
    EV_Data *result = current;
    for(;;)
    {
        if((result - queue->events) >= queue->events_count)
        {
            result = 0;
            break;
        }
        if(!result->is_consumed)
        {
            break;
        }
        result += 1;
    }
    return result;
}

Function EV_Data *
EV_QueueFirstGet(EV_Queue *queue)
{
    EV_Data *result = queue->events;
    result = EV_QueueIncrementNotConsumed_(queue, result);
    return result;
}

Function EV_Data *
EV_QueueNextGet(EV_Queue *queue, EV_Data *current)
{
    EV_Data *result = EV_QueueIncrementNotConsumed_(queue, current + 1);
    return result;
}

//~NOTE(tbt): event loop wrappers

Function Bool
EV_QueueHasKeyEvent(EV_Queue *queue,
                    I_Key key,
                    I_Modifiers modifiers,
                    Bool is_down,
                    Bool should_consume)
{
    Bool result = False;
    for(EV_QueueForEach(queue, e))
    {
        if(EV_Kind_Key == e->kind &&
           key == e->key &&
           (I_Modifiers_ANY == modifiers || modifiers == e->modifiers) &&
           is_down == e->is_down)
        {
            result = True;
            if(should_consume)
            {
                EV_Consume(e);
            }
            G_ForceNextUpdate();
            break;
        }
    }
    return result;
}

Function Bool
EV_QueueHasKeyDown(EV_Queue *queue,
                   I_Key key,
                   I_Modifiers modifiers,
                   Bool should_consume)
{
    Bool result = EV_QueueHasKeyEvent(queue, key, modifiers, True, should_consume);
    return result;
}

Function Bool
EV_QueueHasKeyUp(EV_Queue *queue,
                 I_Key key,
                 I_Modifiers modifiers,
                 Bool should_consume)
{
    Bool result = EV_QueueHasKeyEvent(queue, key, modifiers, False, should_consume);
    return result;
}

Function V2F
EV_QueueScrollGet(EV_Queue *queue)
{
    V2F delta = U2F(0.0f);
    for(EV_QueueForEach(queue, e))
    {
        if(EV_Kind_MouseScroll == e->kind)
        {
            delta = Add2F(delta, e->position);
        }
    }
    return delta;
}
