#pragma once
#include "ImGui/imgui.h"
namespace FluidRender
{
    inline bool GridLines = true;
    inline ImVec4 GridLinesColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    inline ImVec4 TextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    inline bool Density = true;
    inline bool HorizontalMomentum = false;
    inline bool VerticalMomentum = false;
    inline bool TotalEnergy = false;

    
    void Tick();
    void CollapsingHeader();
};

