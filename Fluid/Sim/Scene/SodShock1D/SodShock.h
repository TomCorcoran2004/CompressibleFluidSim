#pragma once
#include <optional>
#include <glfw/glfw3.h>

#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/implot.h>

#include "../../Mesh/Mesh.h"
#include "../../Fluid/Fluid.h"
#include "../../BoundaryRegion/BoundaryRegion.h"

#include "../Interface.h"

bool BorderFunc(const BoundaryRegion::FaceInfo& FaceInfo, const BoundaryRegion::MeshInfo& MeshInfo);
bool FluidFunc(const BoundaryRegion::FaceInfo& FaceInfo, const BoundaryRegion::MeshInfo& MeshInfo);

class SodShock1D : public SceneInterface
{
public:
    struct Config
    {
        i32 NumCells = 0;
    };

    SodShock1D(const Config& SceneConfig);

    void Reset() override;
    void Tick() override;

    void SetPaused(bool Paused) override;
    virtual bool Paused() const override;
    virtual void Step() override;

    virtual bool IsFinished() const override;

    virtual void DrawGui() const override;
    virtual void DrawResults() const override;

    virtual const Mesh& GetMesh() const override;
    virtual const Fluid& GetFluid() const override;
    virtual std::string_view GetName() const override;

private:
    std::string Name = "SodShock1D";

    bool IsPaused = false;

    Mesh SceneMesh;
    std::optional<Fluid> SceneFluid;

    i32 TicksCompleted = 0;


    f32 DeltaTime = 0.0f;
    f32 CurrentTime = 0.0f;
    f32 LastTickTime = 0.0f;
    static constexpr f32 TotalSceneTime = 0.2f;

    void SetConservedState();
};
