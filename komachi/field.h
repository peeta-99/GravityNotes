#pragma once
#include "renderer.h"

struct MODEL;

class Field
{
public:
    void Init();
    void Update();
    void Draw();
    void Finalize();

    // レーンインデックス(-1,0,+1) -> ワールドX座標
    static float LaneToWorldX(int laneIndex) { return laneIndex * LANE_WIDTH; }

    static constexpr float LANE_WIDTH  = 2.0f;   // 1レーンの幅
    static constexpr float TUNNEL_HALF = 3.0f;   // トンネル半幅 (LANE_WIDTH * 3 / 2)
    static constexpr float SEGMENT_LEN = 20.0f;  // セグメント1本の長さ

private:
    void BuildLineMesh();
    void DrawLaneLines(float segCenterZ);

    static constexpr int   SEG_COUNT = 4;
    static constexpr float RECYCLE_Z = -15.0f;  // 前端がここを過ぎたらリサイクル

    MODEL*                    m_pModel    = nullptr;
    float                     m_segZ[SEG_COUNT]{};
    ID3D11Buffer*             m_pLineVB   = nullptr;
    ID3D11ShaderResourceView* m_pWhiteSRV = nullptr;
    int                       m_lineVerts = 0;
};
