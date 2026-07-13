#include "SodShock.h"

bool BorderFunc(const BoundaryRegion::FaceInfo& FaceInfo, const BoundaryRegion::MeshInfo& MeshInfo)
{
    if (FaceInfo.LeftCell % MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.LeftCell % MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.x - 1 ||
        FaceInfo.LeftCell / MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.LeftCell / MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.y - 1)
        return true;

    if (FaceInfo.RightCell % MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.RightCell % MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.x - 1 ||
        FaceInfo.RightCell / MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.RightCell / MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.y - 1)
        return true;

    return false;
}

bool FluidFunc(const BoundaryRegion::FaceInfo& FaceInfo, const BoundaryRegion::MeshInfo& MeshInfo)
{
    if (FaceInfo.LeftCell % MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.LeftCell % MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.x - 1 ||
        FaceInfo.LeftCell / MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.LeftCell / MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.y - 1)
        return false;

    if (FaceInfo.RightCell % MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.RightCell % MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.x - 1 ||
        FaceInfo.RightCell / MeshInfo.MeshResolution.x == 0 ||
        FaceInfo.RightCell / MeshInfo.MeshResolution.x == MeshInfo.MeshResolution.y - 1)
        return false;

    return true;
}

SodShock1D::SodShock1D(const SodShock1D::Config& SceneConfig)
{
    CurrentTime = static_cast<f32>(glfwGetTime());
    LastTickTime = static_cast<f32>(glfwGetTime());
    
    const Mesh::Config MeshConfig = {
        .Resolution = ivec2(SceneConfig.NumCells + 2, 3),
        .Dimensions = vec2(1.0f, 1.0f)
    };
    SceneMesh = Mesh(MeshConfig);

    TotalTicks = TotalTicks;

    const BoundaryRegion::BoundaryConfig BorderConfig = {
        .Name = "Border",
        .Type = BoundaryRegion::BoundaryTypes::SlipWall,
        .FaceInRegion = BorderFunc
    };
    SceneMesh.AddBoundaryRegion(BorderConfig);

    const BoundaryRegion::BoundaryConfig FluidBorderConfig = {
        .Name = "Fluid",
        .Type = BoundaryRegion::BoundaryTypes::None,
        .FaceInRegion = FluidFunc
    };
    SceneMesh.AddBoundaryRegion(FluidBorderConfig);

    Fluid::Config FluidConfig = Fluid::Config{
            .Mesh = &SceneMesh,
            .dt = TotalSceneTime / SceneConfig.TotalTicks,
            .R = 1.0f,
            .gamma = 1.4f
    };
    SceneFluid = Fluid(FluidConfig);

    SetConservedState();
}

void SodShock1D::Reset()
{
    TicksCompleted = 0;
    CurrentTime = glfwGetTime();
    LastTickTime = glfwGetTime();

    SetConservedState();
}

void SodShock1D::Tick()
{
    DeltaTime = CurrentTime - LastTickTime;
    LastTickTime = CurrentTime;
    CurrentTime = glfwGetTime();
    
    if (TicksCompleted <= TotalTicks)
    {
        ++TicksCompleted;

        SceneFluid.Tick();

        DrawGui();
    }
    else
    {
        //DrawGui();
        DrawResults();
    }
}

void SodShock1D::SetPaused(bool Paused)
{
    IsPaused = Paused;
}

bool SodShock1D::Paused() const
{
    return IsPaused;
}

void SodShock1D::Step()
{
    if (Paused() == false) SetPaused(true);

    Tick();
}

bool SodShock1D::IsFinished() const
{
    return TicksCompleted == TotalTicks;
}

void SodShock1D::DrawGui() const
{
    ivec2 CellsSize = SceneMesh.GetCellsSize();
    i32 Start = SceneMesh.GetCellIndex(ivec2(1, 1));
    i32 End = SceneMesh.GetCellIndex(ivec2(CellsSize.x - 1, 1));
    i32 Size = End - Start;

    std::span<const f32> Rho = SceneFluid.GetRho(Start, End);
    std::span<const f32> RhoU = SceneFluid.GetRhoU(Start, End);
    std::span<const f32> RhoV = SceneFluid.GetRhoV(Start, End);
    std::span<const f32> ETotal = SceneFluid.GetETotal(Start, End);

    std::vector<f32> u(Size);
    std::vector<f32> v(Size);
    std::vector<f32> P(Size);
    std::vector<f32> XPos(Size);

    for (i32 i = 0; i < XPos.size(); ++i)
    {
        XPos[i] = (static_cast<f32>(i) + 0.5f) / static_cast<f32>(XPos.size());
    }

    for (i32 i = 0; i < Size; ++i)
    {
        u[i] = RhoU[i] / Rho[i];
        v[i] = RhoV[i] / Rho[i];
        P[i] = (1.4f - 1.0f) * (ETotal[i] - 0.5f * Rho[i] * (u[i] * u[i] + v[i] * v[i]));
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

    ImGui::Text("%d / %d Ticks", TicksCompleted, TotalTicks);
    ImGui::Text("TPS = %.3f s", 1.0f / DeltaTime);
    ImGui::Text("Estimated Time Remaining = %f", (TotalTicks - TicksCompleted) * DeltaTime);

    if (ImPlot::BeginPlot("My Plot", ImVec2(-1, -1)))
    {
        ImPlotAxisFlags AxisFlags =
            ImPlotAxisFlags_NoMenus |
            ImPlotAxisFlags_NoSideSwitch |
            ImPlotAxisFlags_NoHighlight |
            ImPlotAxisFlags_Foreground |
            ImPlotAxisFlags_RangeFit |
            ImPlotAxisFlags_Lock;

        ImPlot::SetupAxes("Position", "Density", AxisFlags, AxisFlags);

        ImPlot::PlotLine("Density", XPos.data(), Rho.data(), XPos.size());
        ImPlot::PlotLine("Velocity", XPos.data(), u.data(), XPos.size());
        ImPlot::PlotLine("Pressure", XPos.data(), P.data(), XPos.size());

        ImPlot::EndPlot();
    }
    ImGui::End();
}


void SodShock1D::DrawResults() const
{

}

const Mesh& SodShock1D::GetMesh() const
{
    return SceneMesh;
}

const Fluid& SodShock1D::GetFluid() const
{
    return SceneFluid;
}

std::string_view SodShock1D::GetName() const
{
    return Name;
}

void SodShock1D::SetConservedState()
{
    for (i32 i = 0; i < SceneMesh.GetCellsSizeFlat(); ++i)
    {
        ivec2 CellPosition = SceneMesh.GetCellPosition(i);

        if (CellPosition.x < SceneMesh.GetCellsSize().x / 2)
        {
            SceneFluid.SetRho(i, 1.0f);
            SceneFluid.SetU(i, 0.0f);
            SceneFluid.SetV(i, 0.0f);
            SceneFluid.SetP(i, 1.0f);
        }
        else
        {
            SceneFluid.SetRho(i, 0.125f);
            SceneFluid.SetU(i, 0.0f);
            SceneFluid.SetV(i, 0.0f);
            SceneFluid.SetP(i, 0.1f);
        }
    }
}

f32 PressureFunction(f32 p, f32 pk, f32 alpha_k, f32 gamma, f32 rho_k)
{
    if (p <= pk)
    {
        return (2.0f * alpha_k) / (gamma - 1.0f) * (glm::pow(p / pk, (gamma - 1.0f) / 2.0f * gamma) - 1.0f);
    }
    else
    {
        f32 ak = 2.0f / (gamma + 1) * rho_k;
        f32 bk = (gamma - 1) / (gamma + 1) * pk;

        return (p - pk) * glm::sqrt(ak / (p + bk));
    }
}

SodShock1D::PrimitiveState SodShock1D::ReimannSolver(f32 Position)
{
    const f32 xi = Position / 0.2f;

    //alpha = sqrt(gamma * P / rho)
    const f32 alpha_l = glm::sqrt(1.4f);
    const f32 alpha_r = glm::sqrt((1.4f * 0.1f) / 0.125);
}
