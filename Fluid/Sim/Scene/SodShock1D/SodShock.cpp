#include "SodShock.h"
#include "RiemannSolver/Riemann.h"

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
            SceneMesh,  //Mesh
            1.0f,       //R
            1.4f,       //Gamma
            0.5f        //Cfl Target
    };

    SceneFluid.emplace(FluidConfig);

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
    
    if (SceneFluid->GetTimeElapsed() < TotalSceneTime)
    {
        SceneFluid->TimeStep();

        DrawGui();
    }
    else
    {
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
    return SceneFluid->GetTimeElapsed() >= TotalSceneTime;
}

void SodShock1D::DrawGui() const
{
    ivec2 CellsSize = SceneMesh.GetCellsSize();
    i32 Start = SceneMesh.GetCellIndex(ivec2(1, 1));
    i32 End = SceneMesh.GetCellIndex(ivec2(CellsSize.x - 1, 1));
    const i32 Size = End - Start;

    std::span<const f32> Rho = SceneFluid->GetRho(Start, End);
    std::span<const f32> RhoU = SceneFluid->GetRhou(Start, End);
    std::span<const f32> RhoV = SceneFluid->GetRhov(Start, End);
    std::span<const f32> ETotal = SceneFluid->GetE(Start, End);

    std::vector<f32> u(Size);
    std::vector<f32> v(Size);
    std::vector<f32> P(Size);
    std::vector<f32> XPos(Size);

    //for (i32 i = 0; i < XPos.size(); ++i)
    //{
    //    XPos[i] = (static_cast<f32>(i) + 0.5f) / static_cast<f32>(XPos.size());
    //}

    //for (i32 i = 0; i < Size; ++i)
    //{
    //    u[i] = RhoU[i] / Rho[i];
    //    v[i] = RhoV[i] / Rho[i];
    //    P[i] = (1.4f - 1.0f) * (ETotal[i] - 0.5f * Rho[i] * (u[i] * u[i] + v[i] * v[i]));
    //}

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

    ImGui::Text("%.3f / %.3f Seconds Elapsed", SceneFluid->GetTimeElapsed(), TotalSceneTime);
    ImGui::Text("TPS = %.3f s", 1.0f / DeltaTime);

    //if (ImPlot::BeginPlot("Sod Shock 1D", ImVec2(-1, -1)))
    //{
    //    ImPlotAxisFlags AxisFlags =
    //        ImPlotAxisFlags_NoMenus |
    //        ImPlotAxisFlags_NoSideSwitch |
    //        ImPlotAxisFlags_NoHighlight |
    //        ImPlotAxisFlags_Foreground |
    //        ImPlotAxisFlags_RangeFit |
    //        ImPlotAxisFlags_Lock;
    //
    //    ImPlot::SetupAxes("Position", "Density", AxisFlags, AxisFlags);
    //
    //    ImPlot::PlotLine("Density", XPos.data(), Rho.data(), XPos.size());
    //    ImPlot::PlotLine("Velocity", XPos.data(), u.data(), XPos.size());
    //    ImPlot::PlotLine("Pressure", XPos.data(), P.data(), XPos.size());
    //
    //    ImPlot::EndPlot();
    //}
    ImGui::End();
}


void SodShock1D::DrawResults() const
{
    ivec2 CellsSize = SceneMesh.GetCellsSize();
    i32 Start = SceneMesh.GetCellIndex(ivec2(1, 1));
    i32 End = SceneMesh.GetCellIndex(ivec2(CellsSize.x - 1, 1));
    i32 Size = End - Start;

    std::span<const f32> Rho = SceneFluid->GetRho(Start, End);
    std::span<const f32> RhoU = SceneFluid->GetRhou(Start, End);
    std::span<const f32> RhoV = SceneFluid->GetRhov(Start, End);
    std::span<const f32> ETotal = SceneFluid->GetE(Start, End);

    std::vector<f32> u(Size);
    std::vector<f32> v(Size);
    std::vector<f32> P(Size);
    std::vector<f32> XPos(Size);

    RiemannSolver::W Left = {
                .rho = 1.0f,
                .u = 0.0f,
                .p = 1.0f
    };

    RiemannSolver::W Right = {
        .rho = 0.125f,
        .u = 0.0f,
        .p = 0.1f
    };

    RiemannSolver::Config RiemannConfig = {
        .Left = Left,
        .Right = Right,
        .gamma = 1.4f
    };

    RiemannSolver Solver = { RiemannConfig };

    std::vector<f32> RhoExact(Size);
    std::vector<f32> UExact(Size);
    std::vector<f32> PExact(Size);

    f32 RhoResidual = 0.0f;
    f32 UResidual = 0.0f;
    f32 PResidual = 0.0f;

    for (i32 i = 0; i < XPos.size(); ++i)
    {
        XPos[i] = (static_cast<f32>(i) + 0.5f) / static_cast<f32>(XPos.size());
    }

    for (i32 i = 0; i < Size; ++i)
    {
        u[i] = RhoU[i] / Rho[i];
        v[i] = RhoV[i] / Rho[i];
        P[i] = (1.4f - 1.0f) * (ETotal[i] - 0.5f * Rho[i] * (u[i] * u[i] + v[i] * v[i]));

        RiemannSolver::W State = Solver.Sample(XPos[i] - 0.5f, 0.2f);
        RhoExact[i] = State.rho;
        UExact[i] = State.u;
        PExact[i] = State.p;

        RhoResidual += glm::abs(RhoExact[i] - Rho[i]) * SceneMesh.Getdx();
        UResidual += glm::abs(UExact[i] - u[i]) * SceneMesh.Getdx();
        PResidual += glm::abs(PExact[i] - P[i]) * SceneMesh.Getdx();
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

    
    ImVec2 AvailableWindowArea = ImGui::GetContentRegionAvail();
    ImVec2 PlotSize = { AvailableWindowArea.x * 0.5f, AvailableWindowArea.y * 0.5f };

    ImPlotAxisFlags AxisFlags =
        ImPlotAxisFlags_NoMenus |
        ImPlotAxisFlags_NoSideSwitch |
        ImPlotAxisFlags_NoHighlight |
        ImPlotAxisFlags_Foreground |
        ImPlotAxisFlags_RangeFit |
        ImPlotAxisFlags_Lock;

    if (ImGui::BeginTable("SodShock Results", 2))
    {
        ImGui::TableNextColumn();
        
        if (ImPlot::BeginPlot("Density", PlotSize))
        {
            ImPlot::SetupAxes("Position", "Density", AxisFlags, AxisFlags);
            ImPlot::PlotLine("Density", XPos.data(), Rho.data(), XPos.size());
            ImPlot::PlotLine("Density Exact", XPos.data(), RhoExact.data(), XPos.size());
            ImPlot::EndPlot();
        }

        if (ImPlot::BeginPlot("Velocity", PlotSize))
        {
            ImPlot::SetupAxes("Position", "Velocity", AxisFlags, AxisFlags);
            ImPlot::PlotLine("Velocity", XPos.data(), u.data(), XPos.size());
            ImPlot::PlotLine("Velocity Exact", XPos.data(), UExact.data(), XPos.size());
            ImPlot::EndPlot();
        }

        ImGui::TableNextColumn();
        
        if (ImPlot::BeginPlot("Pressure", PlotSize))
        {
            ImPlot::SetupAxes("Position", "Pressure", AxisFlags, AxisFlags);
            ImPlot::PlotLine("Pressure", XPos.data(), P.data(), XPos.size());
            ImPlot::PlotLine("Pressure Exact", XPos.data(), PExact.data(), XPos.size());
            ImPlot::EndPlot();
        }

        ImGui::Text("Rho Residual: %.5f", RhoResidual);
        ImGui::Text("U Residual: %.5f", UResidual);
        ImGui::Text("P Residual: %.5f", PResidual);
        
        ImGui::EndTable();
    }

    ImGui::End();
}

const Mesh& SodShock1D::GetMesh() const
{
    return SceneMesh;
}

const Fluid& SodShock1D::GetFluid() const
{
    return *SceneFluid;
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
            SceneFluid->SetRho(i, 1.0f);
            SceneFluid->SetU(i, 0.0f);
            SceneFluid->SetV(i, 0.0f);
            SceneFluid->SetP(i, 1.0f);
        }
        else
        {
            SceneFluid->SetRho(i, 0.125f);
            SceneFluid->SetU(i, 0.0f);
            SceneFluid->SetV(i, 0.0f);
            SceneFluid->SetP(i, 0.1f);
        }
    }
}
