/*定数バッファ C言語から受け取るデータ楊の変数*/
//ワールド行列
cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

//カメラ行列
cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

//プロジェクション行列
cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

cbuffer MaterialBuffer : register(b3)
{
    float4 MaterialAmbient;
    float4 MaterialDiffuse;
    float4 MaterialSpecular;
    float4 MaterialEmission;
    float MaterialShininess;
    float3 MaterialPadding;
}

//頂点構造体 頂点シェーダーが頂点バッファの情報を受け取るための構造体
struct VS_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

//頂点(ピクセル)構造体 頂点シェーダーの出力とピクセルシェーダーの入力を兼ねている
struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float4 ShadowPosition : TEXCOORD1;
};

//ライト構造体 今後使用するライトのデータを受け取る構造体
struct LIGHT
{
    bool Enable;
    bool3 Dummy;
    float4 Direction;
    float4 Diffuse;
    float4 Ambient;
    
    float4 Position;
    float4 PointLightParam;
};

/*その他の定数バッファ*/
//ライトオブジェクト
cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
};

//カメラ座標
cbuffer CameraBuffer : register(b5)
{
    float4 CameraPosition;
};

//汎用パラメーター
cbuffer ParameterBuffer : register(b6)
{
    float4 Parameter;
};

cbuffer ShadowBuffer : register(b8)
{
    matrix LightViewProjection;
    float4 ShadowParam; // x: bias, y: shadow brightness
};

Texture2D g_ShadowMap : register(t1);
SamplerState g_ShadowSampler : register(s1);

float CalcShadow(float4 shadowPosition)
{
    float3 proj = shadowPosition.xyz / shadowPosition.w;
    float2 uv = float2(proj.x * 0.5f + 0.5f, -proj.y * 0.5f + 0.5f);

    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f || proj.z < 0.0f || proj.z > 1.0f)
    {
        return 1.0f;
    }

    float shadowDepth = g_ShadowMap.Sample(g_ShadowSampler, uv).r;
    float currentDepth = proj.z - ShadowParam.x;

    return (currentDepth > shadowDepth) ? ShadowParam.y : 1.0f;
}
