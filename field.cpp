#include "field.h"
#include "model.h"
#include "shadermanager.h"
#include "define.h"
#include "debug_params.h"
#include <cstdio>

using namespace DirectX;

static constexpr const char* TUNNEL_MODEL = "asset/model/tunnel_segment.fbx";

void Field::Init() {
	m_Model = ModelLoad("asset/model/tunnel_segment.fbx");
}

void Field::Update(){

}

void Field::Draw() {
	if (m_Model) {
		ModelDraw(
			m_Model,
			m_Position,
			m_Rotation,
			m_Scale,
			m_Color,
			false,
			S_UNLIT
		);
	}
}

void Field::Finalize() {
	if (m_Model) {
		ModelRelease(m_Model); 
		m_Model = nullptr;
	}
}