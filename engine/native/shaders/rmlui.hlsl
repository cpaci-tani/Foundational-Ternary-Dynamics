// RmlUi UI pipeline (D3D12) — one PSO used by RmlD3D12Renderer::RenderGeometry.
// Textured, vertex-coloured, ortho screen-space, scissor-clipped, no depth (UI
// draws over the 3D scene). RmlUi 6.x colours + the font atlas are PREMULTIPLIED
// alpha, so the PSO blend is ONE / INV_SRC_ALPHA (not SRC_ALPHA). The shader
// body (col * tex) is correct for premultiplied inputs.
//
// Vertex format matches Rml::Vertex:
//   float2 position;          -> POSITION  (R32G32_FLOAT)
//   Rml::Colourb colour;      -> COLOR     (R8G8B8A8_UNORM, RGBA -> float4 [0,1])
//   float2 tex_coord;         -> TEXCOORD0 (R32G32_FLOAT)
//
// Untextured RmlUi geometry (texture handle 0) is drawn with a 1x1 white texture
// bound at t0, so `colour * white == colour`.

cbuffer UiConstants : register(b0)
{
    float4x4 uProj;       // ortho(0, width, height, 0)
    float2   uTranslate;  // per-RenderGeometry translation (Rml passes this)
    float2   _pad;
};

Texture2D    uTex     : register(t0);
SamplerState uSampler : register(s0);

struct VSIn
{
    float2 pos : POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD0;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 col : COLOR;
    float2 uv  : TEXCOORD0;
};

VSOut VSMain(VSIn i)
{
    VSOut o;
    float2 p = i.pos + uTranslate;
    o.pos = mul(uProj, float4(p, 0.0, 1.0));
    o.col = i.col;
    o.uv  = i.uv;
    return o;
}

float4 PSMain(VSOut i) : SV_Target
{
    return i.col * uTex.Sample(uSampler, i.uv);
}
