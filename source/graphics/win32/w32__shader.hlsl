struct VS_INPUT
{
    float2 pos :    POSITION;
    float2 uv  :    TEXCOORD;
    float4 colour : COLOUR;
};

struct PS_INPUT
{
    float4 pos    : SV_POSITION;
    float2 uv     : TEXCOORD;
    float4 colour : COLOUR;
};

cbuffer cbuffer0 : register(b0)
{
    float4x4 u_view_projection;
}

sampler sampler0 : register(s0);

Texture2D<float4> texture0 : register(t0);

PS_INPUT
vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(u_view_projection, float4(input.pos, 0.0f, 1.0f));
    output.uv = input.uv;
    output.colour = input.colour;
    return output;
}

float4
ps(PS_INPUT input) : SV_TARGET
{
    float4 tex = texture0.Sample(sampler0, input.uv);
    float4 modulated = tex*input.colour;
    return modulated;
}