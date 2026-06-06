#include "Common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);

    float4 worldPosition = mul(In.Position, World);

    Out.Position = mul(In.Position, wvp);
    Out.WorldPosition = worldPosition;
    Out.Normal = In.Normal;
    Out.Diffuse = In.Diffuse;
    Out.TexCoord = In.TexCoord;
    Out.ShadowPosition = mul(worldPosition, LightViewProjection);
}
