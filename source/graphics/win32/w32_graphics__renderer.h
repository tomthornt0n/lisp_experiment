
typedef struct W32_Sprite W32_Sprite;
struct W32_Sprite
{
    R_Sprite parent;
    W32_Sprite *free_list_next;
    ID3D11Texture2D *texture; // NOTE(tbt): normally 0 for immutable sprite
    ID3D11ShaderResourceView *texture_view;
};

Global W32_Sprite w32_sprite_nil =
{
    .parent.dimensions = { 1, 1, },
    .free_list_next = &w32_sprite_nil,
};

typedef struct
{
    Semaphore lock;
    M_Arena arena;
    W32_Sprite *free_list;
} W32_SpriteAllocator;
Global W32_SpriteAllocator w32_sprite_allocator = {0};

Function W32_Sprite *W32_SpriteAllocate (void);
Function void        W32_SpriteFree     (W32_Sprite *sprite);

typedef struct
{
    Bool was_error;
    ID3D11VertexShader *vertex_shader;
    ID3D11PixelShader *pixel_shader;
    ID3D11InputLayout *input_layout;
} W32_Shader;

typedef struct
{
    M4x4F u_view_projection;
} W32_UniformBuffer;

typedef struct
{
    W32_Shader default_shader;
    W32_UniformBuffer uniforms;
    ID3D11Buffer *uniform_buffer;
    ID3D11Buffer *vertex_buffer;
    ID3D11Buffer *index_buffer;
    ID3D11SamplerState *sampler;
    ID3D11BlendState *blend_state;
    ID3D11RasterizerState *rasteriser_state;
    ID3D11DepthStencilState *depth_stencil_state;
} W32_RendererData;
Global W32_RendererData w32_r_data = {0};

typedef struct
{
    R_Vertex vertices[R_Batch_MaxVertices];
    size_t vertices_count;
    
    uint16_t indices[R_Batch_MaxIndices];
    size_t indices_count;
    
    W32_Sprite *texture;
    W32_Shader *shader;
    I2F mask;
    
    W32_Window *window;
    
    Bool should_flush;
} W32_RenderBatch;

Function void W32_RenderBatchQuadPush (W32_RenderBatch *batch, I2F rect, I2F uv, V4F colour);
Function void W32_RenderBatchArcPush  (W32_RenderBatch *batch, V2F pos, float r, I1F segment, V4F colour);
Function void W32_RenderBatchFlush    (W32_RenderBatch *batch);

