#pragma once
#include "../Scene.h"
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/implot.h>
#include <glfw/glfw3.h>

class SodShock1D : public SceneBase
{
public:
    SodShock1D() {};

    void Init(i32 _NumCells, f32 _dt)
    {
        dt = _dt;
        NumCells = _NumCells;
        MeshSize = ivec2(NumCells + 2, 3);
        SceneMesh = Mesh(MeshSize);
        SceneMesh.AddRect(ivec2(0, 0), SceneMesh.GetGridSize(), Mesh::CellType::Solid, Mesh::FaceType::SolidSlip);
        
        CurrentTime = glfwGetTime();
        LastTickTime = glfwGetTime();

        Fluid::Config FluidConfig = Fluid::Config{
            .dt = dt,
            .R = 1.0f,
            .gamma = 1.4f
        };

        SceneFluid = Fluid(FluidConfig, SceneMesh);
        i32 Start = NumCells + 3;
        i32 End = Start + NumCells;

        for (i32 i = Start; i < End; ++i)
        {
            if (i - NumCells + 3 < NumCells / 2)
            {
                // Left state: high density, high pressure
                SceneFluid.SetRho(i, 1.0f);
                SceneFluid.SetU(i, 0.0f);
                SceneFluid.SetV(i, 0.0f);
                SceneFluid.SetP(i, 1.0f);
            }
            else
            {
                // Right state: low density, low pressure
                SceneFluid.SetRho(i, 0.125f);
                SceneFluid.SetU(i, 0.0f);
                SceneFluid.SetV(i, 0.0f);
                SceneFluid.SetP(i, 0.1f);
            }
        }

        MaxTicks = 0.2f / FluidConfig.dt;

        Position.resize(NumCells);
        f32 dx = 1.0f / NumCells;

        for (i32 i = 0; i < NumCells; ++i)
        {
            Position[i] = i * dx;
        }
    }

    void Tick() override
    {
        if (Ticks <= MaxTicks)
        {
            SceneFluid.Tick();
            ++Ticks;
        }
        
        Render();
    }

private:
    i32 Ticks = 0;
    i32 MaxTicks = 0;
    f32 dt = 0.0f;
    ivec2 MeshSize = ivec2(0, 0);
    i32 NumCells = 0;
    f64 CurrentTime = 0.0f;
    f64 LastTickTime = 0.0f;

    std::vector<f32> Position;

    void Render()
    {
        std::span<const f32> Rho = SceneFluid.GetRho();
        std::span<const f32> RhoU = SceneFluid.GetRhoU();
        std::span<const f32> RhoV = SceneFluid.GetRhoV();
        std::span<const f32> ETotal = SceneFluid.GetETotal();
        
        std::vector<f32> u(Position.size());
        std::vector<f32> v(Position.size());
        std::vector<f32> P(Position.size());
        
        i32 Start = NumCells + 3;
        i32 End = Start + NumCells;
        for (i32 i = Start, j = 0; i < End; ++i, ++j)
        {
            u[j] = RhoU[i] / Rho[i];
            v[j] = RhoV[i] / Rho[i];

            P[j] = (1.4f - 1.0f)* (ETotal[i] - 0.5f * Rho[i] * (u[j] * u[j] + v[j] * v[j]));
        }
        
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin(
            "Fullscreen",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse
        );
        ImGui::Text("Time = %.3f s", Ticks * dt);
        ImGui::Text("%d / %d Ticks", Ticks, MaxTicks);

        f32 DeltaTime = CurrentTime - LastTickTime;
        LastTickTime = CurrentTime;
        CurrentTime = glfwGetTime();

        ImGui::Text("TPS = %.3f s", 1.0f / DeltaTime);
        ImGui::Text("Estimated Time Remaining = %f", (MaxTicks - Ticks) * DeltaTime);

        if (ImPlot::BeginPlot("My Plot", ImVec2(-1, -1))) 
        {
            ImPlotAxisFlags AxisFlags =
                //ImPlotAxisFlags_None |
                //ImPlotAxisFlags_NoLabel |
                //ImPlotAxisFlags_NoGridLines |
                //ImPlotAxisFlags_NoTickMarks |
                //ImPlotAxisFlags_NoTickLabels |
                //ImPlotAxisFlags_NoInitialFit |
                ImPlotAxisFlags_NoMenus |
                ImPlotAxisFlags_NoSideSwitch |
                ImPlotAxisFlags_NoHighlight |
                //ImPlotAxisFlags_Opposite |
                ImPlotAxisFlags_Foreground |
                //ImPlotAxisFlags_Invert |
                //ImPlotAxisFlags_AutoFit |
                ImPlotAxisFlags_RangeFit |
                //ImPlotAxisFlags_PanStretch |
                //ImPlotAxisFlags_LockMin |
                //ImPlotAxisFlags_LockMax |
                ImPlotAxisFlags_Lock;
                //ImPlotAxisFlags_NoDecorations;
            
            ImPlot::SetupAxes("Position", "Density", AxisFlags, AxisFlags);
            
            ImPlot::PlotLine("Density", Position.data(), Rho.data() + Position.size() + 3, Position.size());
            ImPlot::PlotLine("Pressure", Position.data(), u.data(), Position.size());
            ImPlot::PlotLine("Velocity", Position.data(), P.data(), Position.size());
            
            ImPlot::EndPlot();
        }
        ImGui::End();
    }
};

