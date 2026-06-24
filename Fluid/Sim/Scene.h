#pragma once
#include "Mesh.h"
#include "Fluid.h"

class SceneBase
{
public:
    SceneBase();

    virtual void Tick();
    const Mesh& GetMesh() const;
    const Fluid& GetFluid() const;
protected:
    Mesh SceneMesh;
    Fluid SceneFluid;
};

