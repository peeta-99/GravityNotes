#include "field.h"
#include "model.h"
#include "shadermanager.h"
#include "define.h"
#include "debug_params.h"
#include <cstdio>

using namespace DirectX;

static constexpr const char* TUNNEL_MODEL = "asset/model/tunnel_segment.fbx";

void Field::Init()
{
    // セグメント中心 Z: -10, 10, 30, 50 (カメラ後方1本 + 前方3本)
    for (int i = 0; i < SEG_COUNT; i++)
        m_segZ[i] = static_cast<float>(i - 1) * SEGMENT_LEN + SEGMENT_LEN * 0.5f;

    // モデルが存在する場合のみロード
    if (FILE* f = fopen(TUNNEL_MODEL, "rb")) { fclose(f); m_pModel = ModelLoad(TUNNEL_MODEL); }

    BuildLineMesh();
}

void Field::BuildLineMesh()
{
    // S_UNLIT はテクスチャ必須のため 1x1 白テクスチャを生成
    {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = 1; td.Height = 1;
        td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        const unsigned int white = 0xFFFFFFFF;
        D3D11_SUBRESOURCE_DATA tid{};
        tid.pSysMem = &white;
        tid.SysMemPitch = 4;

        ID3D11Texture2D* pTex = nullptr;
        GetDevice()->CreateTexture2D(&td, &tid, &pTex);
        GetDevice()->CreateShaderResourceView(pTex, nullptr, &m_pWhiteSRV);
        SAFE_RELEASE(pTex);
    }

    // レーン区切り線 (LINE_LIST): 4面 x 2本 = 8本, 各2頂点 -> 16頂点
    // セグメント中心が Z=0 の局所座標で定義
    const float H  = TUNNEL_HALF;
    const float W  = LANE_WIDTH;
    const float e  = 0.01f;                   // Z-fighting 防止オフセット
    const float z0 = -SEGMENT_LEN * 0.5f;
    const float z1 =  SEGMENT_LEN * 0.5f;

    VERTEX_3D verts[] = {
        // 床 (Y = -H): X = -W
        { {-W, -H+e, z0}, {0,1,0}, {1,1,1,1}, {0,0} },
        { {-W, -H+e, z1}, {0,1,0}, {1,1,1,1}, {0,0} },
        // 床 (Y = -H): X = +W
        { { W, -H+e, z0}, {0,1,0}, {1,1,1,1}, {0,0} },
        { { W, -H+e, z1}, {0,1,0}, {1,1,1,1}, {0,0} },
        // 天井 (Y = +H): X = -W
        { {-W, H-e, z0}, {0,-1,0}, {1,1,1,1}, {0,0} },
        { {-W, H-e, z1}, {0,-1,0}, {1,1,1,1}, {0,0} },
        // 天井 (Y = +H): X = +W
        { { W, H-e, z0}, {0,-1,0}, {1,1,1,1}, {0,0} },
        { { W, H-e, z1}, {0,-1,0}, {1,1,1,1}, {0,0} },
        // 左壁 (X = -H): Y = -W
        { {-H+e, -W, z0}, {1,0,0}, {1,1,1,1}, {0,0} },
        { {-H+e, -W, z1}, {1,0,0}, {1,1,1,1}, {0,0} },
        // 左壁 (X = -H): Y = +W
        { {-H+e,  W, z0}, {1,0,0}, {1,1,1,1}, {0,0} },
        { {-H+e,  W, z1}, {1,0,0}, {1,1,1,1}, {0,0} },
        // 右壁 (X = +H): Y = -W
        { { H-e, -W, z0}, {-1,0,0}, {1,1,1,1}, {0,0} },
        { { H-e, -W, z1}, {-1,0,0}, {1,1,1,1}, {0,0} },
        // 右壁 (X = +H): Y = +W
        { { H-e,  W, z0}, {-1,0,0}, {1,1,1,1}, {0,0} },
        { { H-e,  W, z1}, {-1,0,0}, {1,1,1,1}, {0,0} },
    };
    m_lineVerts = static_cast<int>(sizeof(verts) / sizeof(verts[0]));

    D3D11_BUFFER_DESC bd{};
    bd.Usage     = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(verts);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = verts;
    GetDevice()->CreateBuffer(&bd, &init, &m_pLineVB);
}

void Field::Update()
{
    const float dt    = 1.0f / FPS;
    const float speed = D_PARAMS.noteSpeed;

    for (int i = 0; i < SEG_COUNT; i++)
    {
        m_segZ[i] -= speed * dt;

        // 前端 (プレイヤー側) が RECYCLE_Z を過ぎたらリサイクル
        if (m_segZ[i] + SEGMENT_LEN * 0.5f < RECYCLE_Z)
            m_segZ[i] += SEG_COUNT * SEGMENT_LEN;
    }
}

void Field::DrawLaneLines(float segCenterZ)
{
    ID3D11DeviceContext* ctx = GetDeviceContext();

    ShaderManager* sm = GetShader(S_UNLIT);
    ctx->IASetInputLayout(sm->GetVertexLayout());
    ctx->VSSetShader(sm->GetVertexShader(), nullptr, 0);
    ctx->PSSetShader(sm->GetPixelShader(), nullptr, 0);
    ctx->PSSetShaderResources(0, 1, &m_pWhiteSRV);

    SetWorldMatrix(XMMatrixTranslation(0.0f, 0.0f, segCenterZ));

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    const UINT stride = sizeof(VERTEX_3D);
    const UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &m_pLineVB, &stride, &offset);
    ctx->Draw(m_lineVerts, 0);

    // 後続の描画のためにトポロジを戻す
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Field::Draw()
{
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (int i = 0; i < SEG_COUNT; i++)
    {
        if (m_pModel)
        {
            ModelDraw(m_pModel,
                XMFLOAT3(0.0f, 0.0f, m_segZ[i]),
                XMFLOAT3(0.0f, 0.0f, 0.0f),
                XMFLOAT3(1.0f, 1.0f, 1.0f),
                XMFLOAT4(0.25f, 0.25f, 0.28f, 1.0f),
                false,
                S_UNLIT
            );
        }

        DrawLaneLines(m_segZ[i]);
    }
}

void Field::Finalize()
{
    ModelRelease(m_pModel);
    m_pModel = nullptr;
    SAFE_RELEASE(m_pLineVB);
    SAFE_RELEASE(m_pWhiteSRV);
}
