#pragma once
#include "../Fluid/Sim/Scene.h"

namespace Render
{
    bool Init();
    void BeginFrame();
    void Render();
    void EndFrame();
    void Destroy();
}

