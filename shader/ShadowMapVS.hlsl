#include "Common.hlsl"

void main(in VS_IN In, out float4 OutPosition : SV_POSITION)
{
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);

    OutPosition = mul(In.Position, wvp);
}
