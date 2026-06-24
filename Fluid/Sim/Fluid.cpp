#include "Fluid.h"
#include <vector>
#include <OpenGlBase/Debug/Log.h>
#include <omp.h>

Fluid::Fluid() : SceneMesh(nullptr),
                 dt(0.0f),
                 dx(0.0f),
                 dy(0.0f),
                 R(0.0f),
                 gamma(0.0f)
{
    LeftState = {  };
    RightState = {  };
    rho = {  };
    rho_u = {  };
    rho_v = {  };
    e_total = {  };
    rho_temp = {  };
    rho_u_temp = {  };
    rho_v_temp = {  };
    e_total_temp = {  };
}

Fluid::Fluid(const Config& Config, const Mesh& Mesh) : SceneMesh(&Mesh),
                                                       dt(Config.dt),
                                                       dx(1.0f / Mesh.GetGridSize().x),
                                                       dy(1.0f / Mesh.GetGridSize().y),
                                                       R(Config.R),
                                                       gamma(Config.gamma)
{
    i32 GridSizeFlat = SceneMesh->GetGridSizeFlat();
    i32 FacesSize = SceneMesh->GetFacesSize();
    
    if (GridSizeFlat == 0)
    {
        Base::Log::Error("GridSize == 0");
        return;
    }

    rho.resize(GridSizeFlat);
    rho_u.resize(GridSizeFlat);
    rho_v.resize(GridSizeFlat);
    e_total.resize(GridSizeFlat);

    rho_temp.resize(GridSizeFlat);
    rho_u_temp.resize(GridSizeFlat);
    rho_v_temp.resize(GridSizeFlat);
    e_total_temp.resize(GridSizeFlat);

    LeftState.resize(FacesSize);
    RightState.resize(FacesSize);
    
    for (i32 i = 0; i < FacesSize; ++i)
    {
        Mesh::FaceType FaceType = SceneMesh->GetFaceType(i);
        i32 LeftCellIndex = SceneMesh->GetLeftCell(i);
        i32 RightCellIndex = SceneMesh->GetRightCell(i);

        Mesh::CellType LeftCellType = SceneMesh->GetCellType(LeftCellIndex);
        Mesh::CellType RightCellType = SceneMesh->GetCellType(RightCellIndex);

        switch (FaceType)
        {
            case Mesh::FaceType::NoBoundry:
            {
                LeftState[i] = LeftCellIndex;
                RightState[i] = RightCellIndex;
                break;
            }
            case Mesh::FaceType::SolidSlip:
            {
                if (LeftCellType == Mesh::CellType::Solid)
                {
                    LeftState[i] = AddGhostState();
                    RightState[i] = RightCellIndex;
                }                
                else if (RightCellType == Mesh::CellType::Solid)
                {
                    LeftState[i] = LeftCellIndex;
                    RightState[i] = AddGhostState();
                }
            }
        }
    }

    std::fill(rho.begin(), rho.end(), 1.0f);
    std::fill(rho_u.begin(), rho_u.end(), 0.0f);
    std::fill(rho_v.begin(), rho_v.end(), 0.0f);
    std::fill(e_total.begin(), e_total.end(), 101325.0f / (gamma - 1.0f));

    DeltaFlux.resize(SceneMesh->GetFacesSize());
    std::fill(DeltaFlux.begin(), DeltaFlux.end(), vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void Fluid::Tick()
{
    rho_temp = rho;
    rho_u_temp = rho_u;
    rho_v_temp = rho_v;
    e_total_temp = e_total;

    for (i32 i = 0; i < SceneMesh->GetFacesSize(); ++i)
    {
        Mesh::FaceType FaceType = SceneMesh->GetFaceType(i);

        switch (FaceType)
        {
        case Mesh::FaceType::NoBoundry:
        {
            continue;
        }
        case Mesh::FaceType::SolidSlip:
        {
            i32 LeftCellIndex = SceneMesh->GetLeftCell(i);
            i32 RightCellIndex = SceneMesh->GetRightCell(i);

            Mesh::CellType LeftCellType = SceneMesh->GetCellType(LeftCellIndex);
            Mesh::CellType RightCellType = SceneMesh->GetCellType(RightCellIndex);

            i32 LeftStateIndex = LeftState[i];
            i32 RightStateIndex = RightState[i];

            vec2 FaceNormal = SceneMesh->GetNormal(i);

            if (LeftCellType == Mesh::CellType::Solid)
            {
                vec4 GhostState = CreateSolidSlipState(RightStateIndex, FaceNormal);

                rho[LeftStateIndex] = GhostState.x;
                rho_u[LeftStateIndex] = GhostState.y;
                rho_v[LeftStateIndex] = GhostState.z;
                e_total[LeftStateIndex] = GhostState.w;
            }
            else if (RightCellType == Mesh::CellType::Solid)
            {
                vec4 GhostState = CreateSolidSlipState(LeftStateIndex, FaceNormal);

                rho[RightStateIndex] = GhostState.x;
                rho_u[RightStateIndex] = GhostState.y;
                rho_v[RightStateIndex] = GhostState.z;
                e_total[RightStateIndex] = GhostState.w;
            }
        }
        }
        
    }

    #pragma omp parallel for schedule(static)
    for (i32 i = 0; i < SceneMesh->GetFacesSize(); ++i)
    {
        const vec2 FaceNormal = SceneMesh->GetNormal(i);
        const f32 dl = dx * FaceNormal.x + dy * FaceNormal.y;
        
        const i32 LeftStateIdx = LeftState[i];
        const i32 RightStateIdx = RightState[i];
                
        DeltaFlux[i] = RusanovSolver(LeftStateIdx, RightStateIdx, FaceNormal);
    }

    
    for (i32 i = 0; i < SceneMesh->GetFacesSize(); ++i)
    {
        const vec2 FaceNormal = SceneMesh->GetNormal(i);
        const f32 dl = dx * FaceNormal.x + dy * FaceNormal.y;
        const f32 dt_div_dl = dt / dl;
        
        const i32 LeftStateIdx = LeftState[i];
        const i32 RightStateIdx = RightState[i];

        rho_temp[LeftStateIdx] -= DeltaFlux[i].x * dt_div_dl;
        rho_u_temp[LeftStateIdx] -= DeltaFlux[i].y * dt_div_dl;
        rho_v_temp[LeftStateIdx] -= DeltaFlux[i].z * dt_div_dl;
        e_total_temp[LeftStateIdx] -= DeltaFlux[i].w * dt_div_dl;

        rho_temp[RightStateIdx] += DeltaFlux[i].x * dt_div_dl;
        rho_u_temp[RightStateIdx] += DeltaFlux[i].y * dt_div_dl;
        rho_v_temp[RightStateIdx] += DeltaFlux[i].z * dt_div_dl;
        e_total_temp[RightStateIdx] += DeltaFlux[i].w * dt_div_dl;
    }


    rho = rho_temp;
    rho_u = rho_u_temp;
    rho_v = rho_v_temp;
    e_total = e_total_temp;
}

void Fluid::SetRho(i32 Index, f32 Val)
{
    rho[Index] = Val;
}

void Fluid::SetU(i32 Index, f32 Val)
{
    rho_u[Index] = rho[Index] * Val;
}

void Fluid::SetV(i32 Index, f32 Val)
{
    rho_v[Index] = rho[Index] * Val;
}

void Fluid::SetP(i32 Index, f32 Val)
{
    f32 u = rho_u[Index] / rho[Index];
    f32 v = rho_v[Index] / rho[Index];
    
    e_total[Index] = Val / (gamma - 1.0f) + 0.5f * rho[Index] * (u * u + v * v);
}


std::span<const f32> Fluid::GetRho() const { return rho; }
std::span<const f32> Fluid::GetRhoU() const { return rho_u; }
std::span<const f32> Fluid::GetRhoV() const { return rho_v; }
std::span<const f32> Fluid::GetETotal() const { return e_total; }

vec4 Fluid::CreateSolidSlipState(i32 Other, const vec2& Normal)
{
    //Compute Primitives From Existing State
    f32 u = rho_u[Other] / rho[Other];// u = rho_u / u
    f32 v = rho_v[Other] / rho[Other];// v = rho_v / v
    f32 P = (gamma - 1.0f) * (e_total[Other] - 0.5f * rho[Other] * (u * u + v * v)); // (gamma - 1) * (e_total * 1/2 * rho * (u^2 + v^2)

    u += -2.f * Normal.x * u;
    v += -2.f * Normal.y * v;

    //create ghost state
    vec4 GhostState;
    GhostState.x = rho[Other];
    GhostState.y = u * rho[Other];
    GhostState.z = v * rho[Other];
    GhostState.w = P / (gamma - 1.0f) + 0.5f * rho[Other] * (u * u + v * v);

    return GhostState;
}

vec4 Fluid::RusanovSolver(i32 LeftStateIdx, i32 RightStateIdx, const vec2& Normal)
{
    const f32 rho_L = rho[LeftStateIdx];
    const f32 rho_u_L = rho_u[LeftStateIdx];
    const f32 rho_v_L = rho_v[LeftStateIdx];
    const f32 e_total_L = e_total[LeftStateIdx];

    const f32 Invrho_L = 1.0f / rho_L;

    const f32 u_L = rho_u_L * Invrho_L;
    const f32 v_L = rho_v_L * Invrho_L;
    const f32 P_L = (gamma - 1.0f) * (e_total_L - 0.5f * rho_L * (u_L * u_L + v_L * v_L));
    const f32 c_L = glm::sqrt(gamma * P_L * Invrho_L);

    const f32 NormalVelocity_L = u_L * Normal.x + v_L * Normal.y;

    const vec4 FluxLeft = vec4(
        rho_L * NormalVelocity_L, //Mass
        rho_u_L * NormalVelocity_L + P_L * Normal.x, //UMomentum
        rho_v_L * NormalVelocity_L + P_L * Normal.y, //VMomentum
        (e_total_L + P_L) * NormalVelocity_L //Energy
    );

    const f32 rho_R = rho[RightStateIdx];
    const f32 rho_u_R = rho_u[RightStateIdx];
    const f32 rho_v_R = rho_v[RightStateIdx];
    const f32 e_total_R = e_total[RightStateIdx];

    const f32 Invrho_R = 1.0f / rho_R;

    const f32 u_R = rho_u_R * Invrho_R;
    const f32 v_R = rho_v_R * Invrho_R;
    const f32 P_R = (gamma - 1.0f) * (e_total_R - 0.5f * rho_R * (u_R * u_R + v_R * v_R));
    const f32 c_R = glm::sqrt(gamma * P_R * Invrho_R);

    const f32 NormalVelocity_R = u_R * Normal.x + v_R * Normal.y;

    const vec4 FluxRight = vec4(
        rho_R * NormalVelocity_R, //Mass
        rho_u_R * NormalVelocity_R + P_R * Normal.x, //UMomentum
        rho_v_R * NormalVelocity_R + P_R * Normal.y, //VMomentum
        (e_total_R + P_R) * NormalVelocity_R //Energy
    );

    const f32 alpha = glm::max(glm::abs(NormalVelocity_L) + c_L, 
                               glm::abs(NormalVelocity_R) + c_R);

    return vec4(
        (FluxLeft.x + FluxRight.x) * 0.5f - 0.5f * alpha * (rho_R - rho_L),
        (FluxLeft.y + FluxRight.y) * 0.5f - 0.5f * alpha * (rho_u_R - rho_u_L),
        (FluxLeft.z + FluxRight.z) * 0.5f - 0.5f * alpha * (rho_v_R - rho_v_L),
        (FluxLeft.w + FluxRight.w) * 0.5f - 0.5f * alpha * (e_total_R - e_total_L)
    );
}

i32 Fluid::AddGhostState()
{
    rho.resize(rho.size() + 1);
    rho_u.resize(rho_u.size() + 1);
    rho_v.resize(rho_v.size() + 1);
    e_total.resize(e_total.size() + 1);
    rho_temp.resize(rho_temp.size() + 1);
    rho_u_temp.resize(rho_u_temp.size() + 1);
    rho_v_temp.resize(rho_v_temp.size() + 1);
    e_total_temp.resize(e_total_temp.size() + 1);

    return rho.size() - 1;
}


