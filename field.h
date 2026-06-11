#pragma once
#include "renderer.h"
#include "sprite3d.h"

struct MODEL;

class Field: public Sprite3D
{
private:


public:
    void Init();
    void Update();
    void Draw();
    void Finalize();
};
