
typedef enum
{
    UI_CutDir_Left,
    UI_CutDir_Right,
    UI_CutDir_Top,
    UI_CutDir_Bottom,
} UI_CutDir;

Function I2F UI_CutLeft   (I2F *rect, float f);
Function I2F UI_CutRight  (I2F *rect, float f);
Function I2F UI_CutTop    (I2F *rect, float f);
Function I2F UI_CutBottom (I2F *rect, float f);
Function I2F UI_Cut       (I2F *rect, UI_CutDir dir, float f);

Function I2F UI_GetLeft   (I2F rect, float f);
Function I2F UI_GetRight  (I2F rect, float f);
Function I2F UI_GetTop    (I2F rect, float f);
Function I2F UI_GetBottom (I2F rect, float f);
Function I2F UI_Get       (I2F rect, UI_CutDir dir, float f);

//~NOTE(tbt): scrollable rows layouts

typedef struct
{
    I2F rect;
    float scroll;
    float target_scroll;
    float height_per_row;
    size_t rows_count;
} UI_Scrollable;

Function void UI_ScrollableBegin (UI_Scrollable *state, I2F rect, float row_height);
Function I2F  UI_ScrollableRow   (UI_Scrollable *state);

//~NOTE(tbt): text edits

// TODO(tbt): these are single line only at the moment

typedef Bool UI_EditTextFilterHook(S8 to_insert, void *user_data);

Function void UI_EditTextDeleteRange (S8 *s, size_t cap, I1U *selection);
Function Bool UI_EditText            (char buffer[], size_t cap, I1U *selection, size_t *len, Bool should_consume, UI_EditTextFilterHook filter_hook, void *filter_hook_user_data);
Function void UI_DrawS8WithCaret     (R_Font *font, R_MeasuredText mt, V4F colour, I1U *selection);

//~NOTE(tbt): misc.

Function V2F  UI_TextPositionFromRect (R_Font *font, I2F rect, V2F padding);
Function void UI_DrawLineShadow       (Bool flip, I2F rect, float intensity);

//~NOTE(tbt): animation helpers? (not too sure about these)

#define UI_AnimateSlop (0.001f)

Function Bool  UI_AnimateLinearF      (float result[], size_t n, float a[], float b[], double start, double duration);
Function Bool  UI_AnimateExponentialF (float result[], size_t n, float a[], float b[], double start, double duration);
Function Bool  UI_AnimateSmoothF      (float result[], size_t n, float a[], float b[], double start, double duration);
Function float UI_AnimateInOut1F      (float a, float b, double start_time, double in_time, double sustain, double out_time);
Function float UI_AnimateTowards1F    (float a, float b, float speed);
Function void  UI_AnimateTowardsF     (float a[], float b[], size_t n, float speed);
