#include "Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    float4 texColor = g_Texture.Sample(g_SamplerState, In.TexCoord) * In.Diffuse;
    float shadow = CalcShadow(In.ShadowPosition);

    texColor.rgb *= shadow;
    outDiffuse = texColor;
}
