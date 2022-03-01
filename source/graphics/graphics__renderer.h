
//~NOTE(tbt): sprites

typedef struct
{
    V2I dimensions;
} 
R_Sprite;

Function R_Sprite *R_SpriteNil     (void);
Function Bool      R_SpriteIsNil   (R_Sprite *sprite);
Function R_Sprite *R_SpriteMake    (Pixel *data, V2I dimensions);
Function void      R_SpriteDestroy (R_Sprite *sprite);

typedef I2F R_SubSprite;
Function R_SubSprite R_SubSpriteFromSprite (R_Sprite *sprite, I2F region);
Global R_SubSprite r_entire_texture = { .min = { 0.0f, 0.0f }, .max = { 1.0f, 1.0f } };

//~NOTE(tbt): fonts / text

enum
{
    R_Font_AsciiAtlasDimensions = 2048,
    R_Font_AsciiAtlasPackRangeBegin = 32,
    R_Font_AsciiAtlasPackRangeEnd = 128,
    R_Font_AsciiAtlasPackRangeCount = R_Font_AsciiAtlasPackRangeEnd - R_Font_AsciiAtlasPackRangeBegin,
    
    R_Font_GlyphCacheGridSize = 16, // NOTE(tbt): width and height of the grid in cells
    R_Font_GlyphCacheCellSize = 72, // NOTE(tbt): width and height of each grid cell in pixels
    R_Font_GlyphCacheDimensionsInPixels = R_Font_GlyphCacheGridSize*R_Font_GlyphCacheCellSize,
    R_Font_GlyphCacheCellsCount         = R_Font_GlyphCacheGridSize*R_Font_GlyphCacheGridSize,
    R_Font_GlyphCacheBytesPerCell       = R_Font_GlyphCacheCellSize*R_Font_GlyphCacheCellSize*4,
    R_Font_GlyphCacheSizeInBytes        = R_Font_GlyphCacheDimensionsInPixels*R_Font_GlyphCacheDimensionsInPixels*4,
    
    R_Font_GlyphCacheCellUnoccupied = ~((unsigned int)0),
};


typedef struct
{
    unsigned int gi;
    V2F dimensions;
    V2I bearing;
    R_SubSprite uv;
    int advance;
} R_FontGlyphCacheCellInfo;

typedef struct
{
    M_Arena arena;
    
    R_Sprite *ascii_atlas;
    stbtt_packedchar ascii_atlas_info[R_Font_AsciiAtlasPackRangeCount];
    stbtt_fontinfo font_info;
    S8 font_file;
    int v_advance;
    float scale;
    Bool is_kerning_enabled;
    
    R_Sprite *glyph_cache_atlas;
    unsigned char glyph_cache_pixels[R_Font_GlyphCacheSizeInBytes];
    R_FontGlyphCacheCellInfo glyph_cache_info[R_Font_GlyphCacheCellsCount];
} R_Font;

Function R_Font *R_FontMake     (S8 font_data, int size);
Function R_Font *R_FontFromFile (S8 filename, int size);
Function void    R_FontDestroy  (R_Font *font);

typedef struct
{
    I2F bounds;
    V2F position;
    S8 string;
    S32 codepoints;
    
    size_t rects_count;
    struct R_MeasuredTextRect
    {
        I2F bounds;
        I2F closest;
        float base_line;
    } *rects;
} R_MeasuredText;
typedef enum { R_MeasuredTextIndex_None = ~((size_t)0), } R_MeasuredTextIndex;

Function R_MeasuredText      R_MeasureS8                            (M_Arena *arena, R_Font *font, S8 string, V2F position);
Function R_MeasuredTextIndex R_MeasuredTextIndexFromPosition        (R_MeasuredText measured_text, V2F pixel_position);
Function R_MeasuredTextIndex R_MeasuredTextNearestIndexFromPosition (R_MeasuredText measured_text, V2F pixel_position);
Function V2F                 R_AdvanceFromS8                        (R_Font *font, S8 string);

//~NOTE(tbt): commands

typedef enum
{
    R_CmdKind_Clear,
    
    R_CmdKind_DrawSprite,
    R_CmdKind_DrawRectStroke,
    R_CmdKind_DrawS8,
    R_CmdKind_DrawCircleFill,
    R_CmdKind_DrawRoundedRect,
    
    R_CmdKind_SubQueue,
    
    R_CmdKind_MAX,
} R_CmdKind;

typedef struct R_CmdQueue R_CmdQueue;

typedef struct
{
    uint32_t sort_key;
    R_CmdKind kind;
    R_Sprite *sprite;
    R_SubSprite sub_sprite;
    I2F rect;
    V4F colour;
    R_CmdQueue *sub_queue;
    S8 string;
    R_Font *font;
    float size;
    I2F mask;
} R_Cmd;

enum
{
    R_CmdQueue_Cap = 4096,
};
typedef struct R_CmdQueue
{
    R_Cmd cmds[R_CmdQueue_Cap];
    size_t cmds_count;
} R_CmdQueue;

Function void R_CmdPush (R_CmdQueue *q, R_Cmd cmd);

Function void R_LayerPush (unsigned char layer);
Function void R_LayerPop  (void);
#define R_Layer(L) DeferLoop(R_LayerPush(L), R_LayerPop())
#define R_LayerRelative(L) DeferLoop(R_LayerPush((r_layer_stack[r_layer_stack_count - 1]) + (L)), R_LayerPop())

Function void R_MaskPush             (I2F mask);
Function void R_MaskPushIntersecting (I2F mask);
Function void R_MaskPop              (void);
#define R_Mask(M) DeferLoop(R_MaskPush(M), R_MaskPop())
#define R_MaskIntersecting(M) DeferLoop(R_MaskPushIntersecting(M), R_MaskPop())

Function void R_CmdQueueRecordingBegin (R_CmdQueue *q);
Function void R_CmdQueueRecordingEnd   (R_CmdQueue *q);
#define R_CmdQueueRecord(Q) DeferLoop(R_CmdQueueRecordingBegin(Q), R_CmdQueueRecordingEnd(Q))

Function void R_CmdClear           (V4F colour);
Function void R_CmdDrawSprite      (R_Sprite *sprite, I2F rect, V4F colour);
Function void R_CmdDrawSubSprite   (R_Sprite *sprite, R_SubSprite sub_sprite, I2F rect, V4F colour);
Function void R_CmdDrawRectFill    (I2F rect, V4F colour);
Function void R_CmdDrawRectStroke  (I2F rect, float stroke_width, V4F colour);
Function void R_CmdDrawS8          (R_Font *font, S8 string, V2F position, V4F colour);
Function void R_CmdDrawCircleFill  (V2F position, float radius, V4F colour);
Function void R_CmdDrawRoundedRect (I2F rect, float radius, V4F colour);
Function void R_CmdSubQueue        (R_CmdQueue *sub_queue);

Function void R_CmdQueueSort (R_CmdQueue *q);
Function void R_CmdQueueExec (R_CmdQueue *q, W_Handle window);

//~NOTE(tbt): misc.

enum
{
    R_Batch_MaxVertices = 24576,
    R_Batch_MaxIndices = R_Batch_MaxVertices * 2,
    
    R_ArcVertices = 32,
};

typedef struct
{
    V2F position;
    V2F uv;
    V4F colour;
} R_Vertex;
