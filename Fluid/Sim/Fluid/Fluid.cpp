#include "Fluid.h"
#include "../Mesh/Mesh.h"

Fluid::Fluid()
{
    
}

Fluid::Fluid(const Config& Config) : Fluid()
{
    if (Config.Mesh == nullptr) return;

    SceneMesh = Config.Mesh;

    dx = SceneMesh->Getdx();
    dy = SceneMesh->Getdy();

    dt = Config.dt;
    R = Config.R;
    gamma = Config.gamma;

    // Init ConservedState
    i32 NumCells = SceneMesh->GetCellsSizeFlat();
    
    State.rho.resize(NumCells, 0.0f);
    State.rho_u.resize(NumCells, 0.0f);
    State.rho_v.resize(NumCells, 0.0f);
    State.e_total.resize(NumCells, 0.0f);

    TempState.rho.resize(NumCells, 0.0f);
    TempState.rho_u.resize(NumCells, 0.0f);
    TempState.rho_v.resize(NumCells, 0.0f);
    TempState.e_total.resize(NumCells, 0.0f);

    i32 NumFaces = SceneMesh->GetFacesSize();
    DeltaFluxes.Mass.resize(NumFaces, 0.0f);
    DeltaFluxes.u_momentum.resize(NumFaces, 0.0f);
    DeltaFluxes.v_momentum.resize(NumFaces, 0.0f);
    DeltaFluxes.Energy.resize(NumFaces, 0.0f);
}

void Fluid::SetRho(i32 CellIdx, f32 Val)
{
    State.rho[CellIdx] = Val;
}

void Fluid::SetU(i32 CellIdx, f32 Val)
{
    State.rho_u[CellIdx] = Val * State.rho[CellIdx];
}

void Fluid::SetV(i32 CellIdx, f32 Val)
{
    State.rho_v[CellIdx] = Val * State.rho[CellIdx];
}

void Fluid::SetP(i32 CellIdx, f32 Val)
{
    f32 rho = State.rho[CellIdx];
    f32 u = State.rho_u[CellIdx] / rho;
    f32 v = State.rho_v[CellIdx] / rho;
    
    State.e_total[CellIdx] = IdealGasLaw_E(Val, rho, u, v);
}

std::span<const f32> Fluid::GetRho() const
{ 
    return State.rho; 
}

std::span<const f32> Fluid::GetRhoU() const
{ 
    return State.rho_u; 
}

std::span<const f32> Fluid::GetRhoV() const
{ 
    return State.rho_v; 
}

std::span<const f32> Fluid::GetETotal() const
{ 
    return State.e_total; 
}

std::span<const f32> Fluid::GetRho(i32 Start, i32 End) const
{
    return std::span<const f32>(State.rho).subspan(Start, End);
}

std::span<const f32> Fluid::GetRhoU(i32 Start, i32 End) const
{
    return std::span<const f32>(State.rho_u).subspan(Start, End);
}

std::span<const f32> Fluid::GetRhoV(i32 Start, i32 End) const
{
    return std::span<const f32>(State.rho_v).subspan(Start, End);
}

std::span<const f32> Fluid::GetETotal(i32 Start, i32 End) const
{
    return std::span<const f32>(State.e_total).subspan(Start, End);
}

void Fluid::Tick()
{
    TempState = State;

    const std::span<const i32> LeftCells = SceneMesh->GetLeftCells();
    const std::span<const i32> RightCells = SceneMesh->GetRightCells();
    const std::span<const vec2> Normals = SceneMesh->GetNormals();

    for (const BoundaryRegion& Region : SceneMesh->GetBoundaryRegions())
    {
        BoundaryRegion::BoundaryTypes Type = Region.GetType();
        std::span<const i32> Faces = Region.GetFaces();

        switch (Type)
        {
        case(BoundaryRegion::BoundaryTypes::None):
            for (i32 i = 0; i < Faces.size(); ++i)
            {
                const i32 FaceIdx = Faces[i];

                const i32 LeftCell = LeftCells[FaceIdx];
                const i32 RightCell = RightCells[FaceIdx];
                const vec2 Normal = Normals[FaceIdx];

                const ConservedState LeftConservedState = GetConservedState(LeftCell);

                const ConservedState RightConservedState = GetConservedState(RightCell);

                const Flux DeltaFlux = GetDeltaFlux(LeftConservedState, RightConservedState, Normal);
            
                DeltaFluxes.Mass[FaceIdx] = DeltaFlux.Mass;
                DeltaFluxes.u_momentum[FaceIdx] = DeltaFlux.u_momentum;
                DeltaFluxes.v_momentum[FaceIdx] = DeltaFlux.v_momentum;
                DeltaFluxes.Energy[FaceIdx] = DeltaFlux.Energy;
            }
            break;
        case(BoundaryRegion::BoundaryTypes::SlipWall):
            for (i32 i = 0; i < Faces.size(); ++i)
            {
                const i32 FaceIdx = Faces[i];
                DeltaFluxes.Mass[FaceIdx] = 0.0f;
                DeltaFluxes.u_momentum[FaceIdx] = 0.0f;
                DeltaFluxes.v_momentum[FaceIdx] = 0.0f;
                DeltaFluxes.Energy[FaceIdx] = 0.0f;
                
                const i32 LeftCell = LeftCells[FaceIdx];
                const i32 RightCell = RightCells[FaceIdx];
                const vec2 Normal = Normals[FaceIdx];

                const ConservedState LeftConservedState = GetConservedState(LeftCell);
                const ConservedState RightGhostState = GetSlipWallGhostState(LeftConservedState, Normal);
                
                Flux LeftDeltaFlux = GetDeltaFlux(LeftConservedState, RightGhostState, Normal);

                const ConservedState RightConservedState = GetConservedState(RightCell);
                const ConservedState LeftGhostState = GetSlipWallGhostState(RightConservedState, Normal);

                Flux RightDeltaFlux = GetDeltaFlux(LeftGhostState, RightConservedState, Normal);
                
                const f32 dl = dx * Normal.x + dy * Normal.y;
                const f32 dt_div_dl = dt / dl;

                TempState.rho[LeftCell] -= LeftDeltaFlux.Mass * dt_div_dl;
                TempState.rho_u[LeftCell] -= LeftDeltaFlux.u_momentum * dt_div_dl;
                TempState.rho_v[LeftCell] -= LeftDeltaFlux.v_momentum * dt_div_dl;
                TempState.e_total[LeftCell] -= LeftDeltaFlux.Energy * dt_div_dl;

                TempState.rho[RightCell] += RightDeltaFlux.Mass * dt_div_dl;
                TempState.rho_u[RightCell] += RightDeltaFlux.u_momentum * dt_div_dl;
                TempState.rho_v[RightCell] += RightDeltaFlux.v_momentum * dt_div_dl;
                TempState.e_total[RightCell] += RightDeltaFlux.Energy * dt_div_dl;
                
            }
            break;
        }
    }

    for (i32 i = 0; i < SceneMesh->GetFacesSize(); ++i)
    {
        const vec2& Normal = Normals[i];
        const i32 LeftCell = LeftCells[i];
        const i32 RightCell = RightCells[i];
        
        const f32 dl = dx * Normal.x + dy * Normal.y;
        const f32 dt_div_dl = dt / dl;

        TempState.rho[LeftCell] -= DeltaFluxes.Mass[i] * dt_div_dl;
        TempState.rho_u[LeftCell] -= DeltaFluxes.u_momentum[i] * dt_div_dl;
        TempState.rho_v[LeftCell] -= DeltaFluxes.v_momentum[i] * dt_div_dl;
        TempState.e_total[LeftCell] -= DeltaFluxes.Energy[i] * dt_div_dl;

        TempState.rho[RightCell] += DeltaFluxes.Mass[i] * dt_div_dl;
        TempState.rho_u[RightCell] += DeltaFluxes.u_momentum[i] * dt_div_dl;
        TempState.rho_v[RightCell] += DeltaFluxes.v_momentum[i] * dt_div_dl;
        TempState.e_total[RightCell] += DeltaFluxes.Energy[i] * dt_div_dl;
    }

    State = TempState;
}

Fluid::ConservedState Fluid::GetConservedState(i32 CellIdx)
{
    return ConservedState{
        .rho = State.rho[CellIdx],
        .rho_u = State.rho_u[CellIdx],
        .rho_v = State.rho_v[CellIdx],
        .e_total = State.e_total[CellIdx]
    };
}

Fluid::ConservedState Fluid::GetSlipWallGhostState(const ConservedState& State, const vec2& Normal)
{
    const f32 rho = State.rho;
    const f32 u = State.rho_u / rho;
    const f32 v = State.rho_v / rho;

    f32 dot = u * Normal.x + v * Normal.y;

    f32 u_reflected = u - 2.0f * dot * Normal.x;
    f32 v_reflected = v - 2.0f * dot * Normal.y;

    return ConservedState{
        .rho = State.rho,
        .rho_u = u_reflected * rho,
        .rho_v = v_reflected * rho,
        .e_total = State.e_total
    };
}

Fluid::Flux Fluid::GetDeltaFlux(const ConservedState& Left, const ConservedState& Right, const vec2& Normal)
{
    const DerivedState LeftDerivedState = {
            .u = Left.rho_u / Left.rho,
            .v = Left.rho_v / Left.rho,
            .P = IdealGasLaw_P(Left.e_total, Left.rho, LeftDerivedState.u, LeftDerivedState.v),
            .c = std::sqrtf(gamma * LeftDerivedState.P / Left.rho)
    };

    const f32 LeftNormalVelocity = LeftDerivedState.u * Normal.x + LeftDerivedState.v * Normal.y;

    const Flux LeftFlux = {
        .Mass = Left.rho * LeftNormalVelocity,
        .u_momentum = Left.rho_u * LeftNormalVelocity + LeftDerivedState.P * Normal.x,
        .v_momentum = Left.rho_v * LeftNormalVelocity + LeftDerivedState.P * Normal.y,
        .Energy = (Left.e_total + LeftDerivedState.P) * LeftNormalVelocity
    };

    const DerivedState RightDerivedState = {
        .u = Right.rho_u / Right.rho,
        .v = Right.rho_v / Right.rho,
        .P = IdealGasLaw_P(Right.e_total, Right.rho, RightDerivedState.u, RightDerivedState.v),
        .c = std::sqrtf(gamma * RightDerivedState.P / Right.rho)
    };


    const f32 RightNormalVelocity = RightDerivedState.u * Normal.x + RightDerivedState.v * Normal.y;

    const Flux RightFlux = {
        .Mass = Right.rho * RightNormalVelocity,
        .u_momentum = Right.rho_u * RightNormalVelocity + RightDerivedState.P * Normal.x,
        .v_momentum = Right.rho_v * RightNormalVelocity + RightDerivedState.P * Normal.y,
        .Energy = (Right.e_total + RightDerivedState.P) * RightNormalVelocity
    };

    const f32 alpha = glm::max(glm::abs(LeftNormalVelocity) + LeftDerivedState.c,
        glm::abs(RightNormalVelocity) + RightDerivedState.c);

    Flux DeltaFlux = {
        .Mass =       Rusanov(LeftFlux.Mass,       RightFlux.Mass,       alpha, Left.rho,     Right.rho),
        .u_momentum = Rusanov(LeftFlux.u_momentum, RightFlux.u_momentum, alpha, Left.rho_u,   Right.rho_u),
        .v_momentum = Rusanov(LeftFlux.v_momentum, RightFlux.v_momentum, alpha, Left.rho_v,   Right.rho_v),
        .Energy =     Rusanov(LeftFlux.Energy,     RightFlux.Energy,     alpha, Left.e_total, Right.e_total),
    };

    return DeltaFlux;
}

f32 Fluid::IdealGasLaw_E(f32 P, f32 rho, f32 u, f32 v)
{
    return P / (gamma - 1) + 0.5f * rho * (u * u + v * v);
}

f32 Fluid::IdealGasLaw_P(f32 E, f32 rho, f32 u, f32 v)
{
    return (gamma - 1) * (E - 0.5f * rho * (u * u + v * v));
}

f32 Fluid::Rusanov(f32 FluxLeft, f32 FluxRight, f32 alpha, f32 ConservedLeft, f32 ConservedRight)
{
    return (FluxLeft + FluxRight) * 0.5f - 0.5f * alpha * (ConservedRight - ConservedLeft);
}
