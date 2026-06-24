#include "Scene.h"

SceneBase::SceneBase(){}
void SceneBase::Tick(){}

const Mesh& SceneBase::GetMesh() const 
{
    return SceneMesh;
}

const Fluid& SceneBase::GetFluid() const 
{
    return SceneFluid;
}