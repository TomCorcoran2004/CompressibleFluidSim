#pragma once
#include "../Mesh/Mesh.h"
#include "../Fluid/Fluid.h"

class SceneInterface
{
public:
    virtual ~SceneInterface() = default;

    
    virtual void Reset() = 0;
    virtual void Tick() = 0;

    virtual void SetPaused(bool Paused) = 0;
    virtual bool Paused() const = 0;
    virtual void Step() = 0;
    
    virtual bool IsFinished() const = 0;

    virtual void DrawGui() const = 0;
    virtual void DrawResults() const = 0;

    virtual const Mesh& GetMesh() const = 0;
    virtual const Fluid& GetFluid() const = 0;
    virtual std::string_view GetName() const = 0;
};

