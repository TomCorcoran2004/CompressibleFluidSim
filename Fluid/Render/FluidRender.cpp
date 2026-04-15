#include "FluidRender.h"
#include <vector>
#include <string>
#include <format>
#include <ImGui/imgui.h>
#include <OpenGLBase/Window/Window.h>
#include "../Sim/Sim.h"

namespace FluidRender
{
    void DrawScalarQuantity(const std::vector<f32>& Quantity, u32 TextColor, const ivec2& GridSize, const ivec2& WindowSize)
    {
        if (Quantity.size() != GridSize.x * GridSize.y)
        {
            // TODO: Log error
            return;
        }

        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        ImFont* font = ImGui::GetFont();

        f32 font_size = WindowSize.y / (float)GridSize.y * 0.4f; // scale to 80% of cell height


        f32 cell_width = WindowSize.x / (float)GridSize.x;
        f32 cell_height = WindowSize.y / (float)GridSize.y;

        for (i32 y = 0; y < GridSize.y; ++y)
        {
            for (i32 x = 0; x < GridSize.x; ++x)
            {
                i32 i = y * GridSize.x + x;

                std::string text = std::format("{:.{}f}", Quantity[i], 2);

                ImVec2 text_size = ImGui::CalcTextSize(text.c_str(), nullptr, false, 0.0f);

                // Compute top-left position of the text inside the cell
                ImVec2 pos;
                pos.x = x * cell_width + (cell_width - text_size.x) * 0.5f;
                pos.y = y * cell_height + (cell_height - text_size.y) * 0.5f;

                // Draw the text
                draw_list->AddText(font, font_size, pos, TextColor, text.c_str());
            }
        }
    }
    
    void DrawGridLines(const ivec2& GridSize, const ivec2& WindowSize)
    {
        vec2 CellSize = (vec2)WindowSize / (vec2)GridSize;
        u32 GridLinesColoru32 = ImGui::ColorConvertFloat4ToU32(GridLinesColor);

        for (i32 x = 0; x < GridSize.x; ++x)
        {
            ImVec2 Start = ImVec2(x * CellSize.x, 0);
            ImVec2 End = ImVec2(x * CellSize.x, WindowSize.y);
            ImGui::GetBackgroundDrawList()->AddLine(Start, End, GridLinesColoru32);
        }

        for (i32 y = 0; y < GridSize.y; ++y)
        {
            ImVec2 Start = ImVec2(0, y * CellSize.y);
            ImVec2 End = ImVec2(WindowSize.x, y * CellSize.y);
            ImGui::GetBackgroundDrawList()->AddLine(Start, End, GridLinesColoru32);
        }

        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(0, 0), ImVec2(WindowSize.x, WindowSize.x), GridLinesColoru32);
    }
    
    void Tick()
    {
        if (GridLines) DrawGridLines(Sim::Config::GridSize, Base::Window::GetFrameBufferSize());


        if (Density) DrawScalarQuantity(Sim::rho, ImGui::ColorConvertFloat4ToU32(TextColor), Sim::Config::GridSize, Base::Window::GetWindowSize());
        if (HorizontalMomentum) DrawScalarQuantity(Sim::rho_u, ImGui::ColorConvertFloat4ToU32(TextColor), Sim::Config::GridSize, Base::Window::GetWindowSize());
        if (VerticalMomentum) DrawScalarQuantity(Sim::rho_v, ImGui::ColorConvertFloat4ToU32(TextColor), Sim::Config::GridSize, Base::Window::GetWindowSize());
        if (TotalEnergy) DrawScalarQuantity(Sim::e_total, ImGui::ColorConvertFloat4ToU32(TextColor), Sim::Config::GridSize, Base::Window::GetWindowSize());
    }

    void CollapsingHeader()
    {
        ImGui::Checkbox("Draw GridLines", &GridLines);
        ImGui::ColorEdit4("GridLines Color", (float*)&GridLinesColor);
        ImGui::ColorEdit4("Text Color", (float*)&TextColor);
        ImGui::Checkbox("Draw Density", &Density);
        ImGui::Checkbox("Draw Horizontal Momentum", &HorizontalMomentum);
        ImGui::Checkbox("Draw Vertical Momentum", &VerticalMomentum);
        ImGui::Checkbox("Draw Total Energy", &TotalEnergy);
    }
}
