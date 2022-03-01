
//~NOTE(tbt): event data

typedef enum
{
    EV_Kind_None,
    EV_Kind_Key,
    EV_Kind_Char, // TODO(tbt): implement me!
    EV_Kind_MouseMove,
    EV_Kind_MouseScroll,
    EV_Kind_MouseLeave,
    EV_Kind_WindowSize,
    EV_Kind_MAX,
} EV_Kind;

#define EV_S8FromKind(K) \
(EV_Kind_None        == (K) ? S8("None")          : \
EV_Kind_Key         == (K) ? S8("Key")           : \
EV_Kind_Char        == (K) ? S8("Char")          : \
EV_Kind_MouseMove   == (K) ? S8("Mouse Move")    : \
EV_Kind_MouseScroll == (K) ? S8("Mouse Scroll")  : \
EV_Kind_MouseLeave  == (K) ? S8("Mouse Leave")   : \
EV_Kind_WindowSize  == (K) ? S8("Window Resize") : S8("ERROR - NOT AN EVENT KIND"))

typedef struct
{
    Bool is_consumed;
    EV_Kind kind;
    I_Modifiers modifiers;
    I_Key key;
    Bool is_down;
    V2I size;
    V2F position;
    unsigned int codepoint;
} EV_Data;

Function void EV_Consume (EV_Data *event);

//~NOTE(tbt): event queues

typedef struct
{
    M_Arena arena;
    EV_Data *events;
    size_t events_count;
} EV_Queue;

Function EV_Queue EV_QueueMake    (void);
Function void     EV_QueuePush    (EV_Queue *queue, EV_Data event);
Function void     EV_QueueClear   (EV_Queue *queue);
Function void     EV_QueueDestroy (EV_Queue *queue);

Function EV_Data *EV_QueueFirstGet (EV_Queue *queue);
Function EV_Data *EV_QueueNextGet  (EV_Queue *queue, EV_Data *current);
#define EV_QueueForEach(Q, V) EV_Data *(V) = EV_QueueFirstGet(Q); 0 != (V); (V) = EV_QueueNextGet((Q), (V))

//~NOTE(tbt): event loop wrappers

Function Bool EV_QueueHasKeyEvent (EV_Queue *queue, I_Key key, I_Modifiers modifiers, Bool is_down, Bool should_consume);
Function Bool EV_QueueHasKeyDown  (EV_Queue *queue, I_Key key, I_Modifiers modifiers, Bool should_consume);
Function Bool EV_QueueHasKeyUp    (EV_Queue *queue, I_Key key, I_Modifiers modifiers, Bool should_consume);
Function V2F  EV_QueueScrollGet   (EV_Queue *queue);
