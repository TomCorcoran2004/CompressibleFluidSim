#pragma once
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "../Fluid/DataTypes/StructOfVectors.h"

#include "../Mesh/Mesh.h"

class Fluid
{
public:
    struct Config
    {
        Config(const Mesh& _Mesh, f32 _R, f32 _Gamma, f32 _CflTarget) :
            SceneMesh(_Mesh), R(_R), Gamma(_Gamma), CflTarget(_CflTarget){}
        
        const Mesh& SceneMesh;
        f32 R = 1.0f;
        f32 Gamma = 1.4f;
        f32 CflTarget = 0.5f;
    };

    enum class ConservedFields : std::size_t
    {
        Rho,
        Rhou,
        Rhov,
        E
    };

    enum class DerivedFields : std::size_t
    {
        InvRho,
        u,
        v,
        p,
        c,
    };

    enum class FluxFields : std::size_t
    {
        Mass,
        Momentumu,
        Momentumv,
        Energy,
    };

    Fluid(const Config& Config);    


    void TimeStep();

    //Math Helpers
    f32 IdealGasLaw_E(f32 p, f32 Rho, f32 u, f32 v) const;
    f32 IdealGasLaw_P(f32 E, f32 Rho, f32 u, f32 v) const;
    f32 CflCondition(f32 dx, f32 u, f32 a) const;
    f32 Rusanov(f32 FluxLeft, f32 FluxRight, f32 Alpha, f32 ConservedLeft, f32 ConservedRight) const;

    void SetRho(i32 CellIdx, f32 Val);
    void SetU(i32 CellIdx, f32 Val);
    void SetV(i32 CellIdx, f32 Val);
    void SetP(i32 CellIdx, f32 Val);

    std::span<const f32> GetRho() const;
    std::span<const f32> GetRhou() const;
    std::span<const f32> GetRhov() const;
    std::span<const f32> GetE() const;

    std::span<const f32> GetRho(i32 Start, i32 Count) const;
    std::span<const f32> GetRhou(i32 Start, i32 Count) const;
    std::span<const f32> GetRhov(i32 Start, i32 Count) const;
    std::span<const f32> GetE(i32 Start, i32 Count) const;

    f32 GetGamma() const;
    f32 GetR() const;
    f32 GetTimeElapsed() const;

private:
    SoV<f32, ConservedFields, 4, 64> ConservedStates;
    SoV<f32, ConservedFields, 4, 64> ConservedStatesTemp;
    SoV<f32, DerivedFields, 5, 64> DerivedStates;

    const Mesh& SceneMesh;

    f32 dt;
    f32 R;
    f32 Gamma;
    f32 CflTarget;

    f32 TimeElapsed;

    void CalculateDerivedStates();
    void CalculateTimeStep();
    void CalculateFaceFluxTransfer();

    void CalculateNoneRegion(const BoundaryRegion& Region);
    void CalculateSlipWallRegion(const BoundaryRegion& Region);
    void CalculateSupersonicInflowRegion(const BoundaryRegion& Region);
    void CalculateSubsonicInflowRegion(const BoundaryRegion& Region);
    void CalculateSupersonicOutflowRegion(const BoundaryRegion& Region);
    void CalculateSubsonicOutflowRegion(const BoundaryRegion& Region);
};

