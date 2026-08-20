#include "Fluid.h"

Fluid::Fluid(const Config& Config) : SceneMesh(Config.SceneMesh)
{
    R = Config.R;
    Gamma = Config.Gamma;
    CflTarget = Config.CflTarget;

    // Init ConservedState
    i32 NumCells = SceneMesh.GetCellsSizeFlat();
    ConservedStates.resize(NumCells, 0.0f);
    ConservedStatesTemp.resize(NumCells, 0.0f);
    DerivedStates.resize(NumCells, 0.0f);
}

void Fluid::TimeStep()
{
    CalculateDerivedStates();
    CalculateTimeStep();

    TimeElapsed += dt;

    std::copy(ConservedStates.begin(), ConservedStates.end(), ConservedStatesTemp.begin());

    CalculateFaceFluxTransfer();

    ConservedStates = std::move(ConservedStatesTemp);
}

f32 Fluid::IdealGasLaw_E(f32 P, f32 rho, f32 u, f32 v) const
{
    return P / (Gamma - 1.0f) + 0.5f * rho * (u * u + v * v);
}

f32 Fluid::IdealGasLaw_P(f32 E, f32 rho, f32 u, f32 v) const
{
    return (Gamma - 1.0f) * (E - 0.5f * rho * (u * u + v * v));
}

f32 Fluid::CflCondition(f32 dx, f32 u, f32 a) const
{
    return (CflTarget * dx) / (glm::abs(u) + a);
}

f32 Fluid::Rusanov(f32 FluxLeft, f32 FluxRight, f32 Alpha, f32 ConservedLeft, f32 ConservedRight) const
{
    return (FluxLeft + FluxRight) * 0.5f - 0.5f * Alpha * (ConservedRight - ConservedLeft);
}

void Fluid::SetRho(i32 CellIdx, f32 Val)
{
    ConservedStates[ConservedFields::Rho][CellIdx] = Val;
    DerivedStates[DerivedFields::InvRho][CellIdx] = 1.0f / Val;
}

void Fluid::SetU(i32 CellIdx, f32 Val)
{
    ConservedStates[ConservedFields::Rhou][CellIdx] = Val * ConservedStates[ConservedFields::Rho][CellIdx];
}

void Fluid::SetV(i32 CellIdx, f32 Val)
{
    ConservedStates[ConservedFields::Rhov][CellIdx] = Val * ConservedStates[ConservedFields::Rho][CellIdx];
}

void Fluid::SetP(i32 CellIdx, f32 Val)
{
    f32 u = ConservedStates[ConservedFields::Rhou][CellIdx] * DerivedStates[DerivedFields::InvRho][CellIdx];
    f32 v = ConservedStates[ConservedFields::Rhov][CellIdx] * DerivedStates[DerivedFields::InvRho][CellIdx];

    ConservedStates[ConservedFields::E][CellIdx] = IdealGasLaw_E(Val, ConservedStates[ConservedFields::Rho][CellIdx], u, v);
}

std::span<const f32> Fluid::GetRho() const
{
    return ConservedStates[ConservedFields::Rho];
}

std::span<const f32> Fluid::GetRhou() const
{
    return ConservedStates[ConservedFields::Rhou];
}

std::span<const f32> Fluid::GetRhov() const
{
    return ConservedStates[ConservedFields::Rhov];
}

std::span<const f32> Fluid::GetE() const
{
    return ConservedStates[ConservedFields::E];
}

std::span<const f32> Fluid::GetRho(i32 Start, i32 Count) const
{
    return std::span<const f32>(ConservedStates[ConservedFields::Rho]).subspan(Start, Count);
}

std::span<const f32> Fluid::GetRhou(i32 Start, i32 Count) const
{
    return std::span<const f32>(ConservedStates[ConservedFields::Rhou]).subspan(Start, Count);
}

std::span<const f32> Fluid::GetRhov(i32 Start, i32 Count) const
{
    return std::span<const f32>(ConservedStates[ConservedFields::Rhov]).subspan(Start, Count);
}

std::span<const f32> Fluid::GetE(i32 Start, i32 Count) const
{
    return std::span<const f32>(ConservedStates[ConservedFields::E]).subspan(Start, Count);
}

f32 Fluid::GetGamma() const
{
    return Gamma;
}

f32 Fluid::GetR() const
{
    return R;
}

f32 Fluid::GetTimeElapsed() const
{
    return TimeElapsed;
}

//TODO -> MultiThread
void Fluid::CalculateDerivedStates()
{
    using enum ConservedFields;
    using enum DerivedFields;

    //has to be taken out of the loop for vectorisation
    const std::size_t NumCells = SceneMesh.GetCellsSizeFlat();

    for (std::size_t i = 0; i < NumCells; ++i)
    {
        DerivedStates[InvRho][i] = 1.0f / ConservedStates[Rho][i];
        DerivedStates[v][i] = ConservedStates[Rhov][i] * DerivedStates[InvRho][i];
        DerivedStates[u][i] = ConservedStates[Rhou][i] * DerivedStates[InvRho][i];
        DerivedStates[p][i] = IdealGasLaw_P(ConservedStates[E][i], ConservedStates[Rho][i], DerivedStates[u][i], DerivedStates[v][i]);
        DerivedStates[c][i] = std::sqrtf(Gamma * DerivedStates[p][i] * DerivedStates[InvRho][i]);
    }
}

void Fluid::CalculateTimeStep()
{
    using enum DerivedFields;

    f32 MinTimeStep = std::numeric_limits<f32>::max();

    const f32 dy = SceneMesh.Getdy();
    const f32 dx = SceneMesh.Getdx();

    for (i32 i = 0; i < SceneMesh.GetCellsSizeFlat(); ++i)
    {
        const f32 CflTimeStepX = CflCondition(dx, DerivedStates[u][i], DerivedStates[c][i]);
        const f32 CflTimeStepY = CflCondition(dy, DerivedStates[v][i], DerivedStates[c][i]);

        const f32 CflTimeStep = glm::min(CflTimeStepX, CflTimeStepY);
        MinTimeStep = glm::min(MinTimeStep, CflTimeStep);
    }

    dt = MinTimeStep;
}

void Fluid::CalculateFaceFluxTransfer()
{
    for (const BoundaryRegion& Region : SceneMesh.GetBoundaryRegions())
    {
        BoundaryRegion::BoundaryTypes Type = Region.GetType();
        std::span<const i32> Faces = Region.GetFaces();

        switch (Type)
        {
            case(BoundaryRegion::BoundaryTypes::None): 
            {
                CalculateNoneRegion(Region);
                break;
            }
            case(BoundaryRegion::BoundaryTypes::SlipWall):
            {
                CalculateSlipWallRegion(Region);
                break;
            }
            case(BoundaryRegion::BoundaryTypes::SupersonicInflow):
            {
                CalculateSupersonicInflowRegion(Region);
                break;
            }
            case(BoundaryRegion::BoundaryTypes::SubsonicInflow):
            {
                CalculateSubsonicInflowRegion(Region);
                break;
            }
            case(BoundaryRegion::BoundaryTypes::SupersonicOutflow):
            {
                CalculateSupersonicOutflowRegion(Region);
                break;
            }
            case(BoundaryRegion::BoundaryTypes::SubsonicOutflow):
            {
                CalculateSubsonicOutflowRegion(Region);
                break;
            }
        }
    }
}

void Fluid::CalculateNoneRegion(const BoundaryRegion& Region)
{
    using enum ConservedFields;
    using enum DerivedFields;
    using enum FluxFields;

    const std::span<const i32> Faces = Region.GetFaces();

    for (std::size_t i = 0; i < Faces.size(); ++i)
    {
        const i32 FaceIdx = Faces[i];
        const i32 LeftState = SceneMesh.GetLeftCell(FaceIdx);
        const i32 RightState = SceneMesh.GetRightCell(FaceIdx);
        const vec2 Normal = SceneMesh.GetNormal(FaceIdx);
        const f32 dt_div_dl = dt * SceneMesh.GetInvdl(FaceIdx);

        const f32 LeftNormalVelocity = DerivedStates[u][LeftState] * Normal.x + DerivedStates[v][LeftState] * Normal.y;

        const f32 LeftFlux[4] = {
            ConservedStates[Rho][LeftState] * LeftNormalVelocity,
            ConservedStates[Rhou][LeftState] * LeftNormalVelocity + DerivedStates[p][LeftState] * Normal.x,
            ConservedStates[Rhov][LeftState] * LeftNormalVelocity + DerivedStates[p][LeftState] * Normal.y,
            (ConservedStates[E][LeftState] + DerivedStates[p][LeftState]) * LeftNormalVelocity
        };

        const f32 RightNormalVelocity = DerivedStates[u][RightState] * Normal.x + DerivedStates[v][RightState] * Normal.y;

        const f32 RightFlux[4] = {
            ConservedStates[Rho][RightState] * RightNormalVelocity,
            ConservedStates[Rhou][RightState] * RightNormalVelocity + DerivedStates[p][RightState] * Normal.x,
            ConservedStates[Rhov][RightState] * RightNormalVelocity + DerivedStates[p][RightState] * Normal.y,
            (ConservedStates[E][RightState] + DerivedStates[p][RightState]) * RightNormalVelocity
        };

        const f32 alpha = glm::max(glm::abs(LeftNormalVelocity) + DerivedStates[c][LeftState],
            glm::abs(RightNormalVelocity) + DerivedStates[c][RightState]);

        const f32 DeltaFlux[4] =
        {
            Rusanov(LeftFlux[0], RightFlux[0], alpha, ConservedStates[Rho][LeftState], ConservedStates[Rho][RightState]),
            Rusanov(LeftFlux[1], RightFlux[1], alpha, ConservedStates[Rhou][LeftState], ConservedStates[Rhou][RightState]),
            Rusanov(LeftFlux[2], RightFlux[2], alpha, ConservedStates[Rhov][LeftState], ConservedStates[Rhov][RightState]),
            Rusanov(LeftFlux[3], RightFlux[3], alpha, ConservedStates[E][LeftState], ConservedStates[E][RightState]),
        };

        ConservedStatesTemp[Rho][LeftState] -= DeltaFlux[0] * dt_div_dl;
        ConservedStatesTemp[Rhou][LeftState] -= DeltaFlux[1] * dt_div_dl;
        ConservedStatesTemp[Rhov][LeftState] -= DeltaFlux[2] * dt_div_dl;
        ConservedStatesTemp[E][LeftState] -= DeltaFlux[3] * dt_div_dl;

        ConservedStatesTemp[Rho][RightState] += DeltaFlux[0] * dt_div_dl;
        ConservedStatesTemp[Rhou][RightState] += DeltaFlux[1] * dt_div_dl;
        ConservedStatesTemp[Rhov][RightState] += DeltaFlux[2] * dt_div_dl;
        ConservedStatesTemp[E][RightState] += DeltaFlux[3] * dt_div_dl;
    }
}

void Fluid::CalculateSlipWallRegion(const BoundaryRegion& Region)
{
    using enum ConservedFields;
    using enum DerivedFields;
    using enum FluxFields;
    
    const i32* __restrict Faces = Region.GetFaces().data();
    const i32* __restrict LeftCells = SceneMesh.GetLeftCells().data();
    const i32* __restrict RightCells = SceneMesh.GetRightCells().data();
    const vec2* __restrict Normals = SceneMesh.GetNormals().data();
    const f32* __restrict Invdls = SceneMesh.GetInvdls().data();

    const std::size_t Size = Region.GetFaces().size();
    
    for (std::size_t i = 0; i < Size; ++i)
    {
        const i32 FaceIdx = Faces[i];

        const i32 LeftState = LeftCells[FaceIdx];
        const i32 RightState = RightCells[FaceIdx];
        const vec2 Normal = Normals[FaceIdx];
        const f32 dt_div_dl = dt * Invdls[FaceIdx];

        ConservedStatesTemp[Rhou][LeftState] -= DerivedStates[p][LeftState] * Normal.x * dt_div_dl;
        ConservedStatesTemp[Rhov][LeftState] -= DerivedStates[p][LeftState] * Normal.y * dt_div_dl;
        ConservedStatesTemp[Rhou][RightState] += DerivedStates[p][RightState] * Normal.x * dt_div_dl;
        ConservedStatesTemp[Rhov][RightState] += DerivedStates[p][RightState] * Normal.y * dt_div_dl;
    }
}

void Fluid::CalculateSupersonicInflowRegion(const BoundaryRegion& Region){}
void Fluid::CalculateSubsonicInflowRegion(const BoundaryRegion& Region){}
void Fluid::CalculateSupersonicOutflowRegion(const BoundaryRegion& Region){}
void Fluid::CalculateSubsonicOutflowRegion(const BoundaryRegion& Region){}

