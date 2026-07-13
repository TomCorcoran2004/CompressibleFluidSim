#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../BoundaryRegion/BoundaryRegion.h"

class Mesh;

class Fluid
{
public:
    struct Config
    {
        const Mesh* Mesh = nullptr;
        f32 dt = 0.0f;
        f32 R = 1.0f;
        f32 gamma = 1.4f;
    };

    Fluid();
    Fluid(const Config& Config);

    //EOS's
    f32 IdealGasLaw_E(f32 P, f32 rho, f32 u, f32 v);
    f32 IdealGasLaw_P(f32 E, f32 rho, f32 u, f32 v);

    void SetRho(i32 CellIdx, f32 Val);
    void SetU(i32 CellIdx, f32 Val);
    void SetV(i32 CellIdx, f32 Val);
    void SetP(i32 CellIdx, f32 Val);

    std::span<const f32> GetRho() const;
    std::span<const f32> GetRhoU() const;
    std::span<const f32> GetRhoV() const;
    std::span<const f32> GetETotal() const;

    std::span<const f32> GetRho(i32 Start, i32 End) const;
    std::span<const f32> GetRhoU(i32 Start, i32 End) const;
    std::span<const f32> GetRhoV(i32 Start, i32 End) const;
    std::span<const f32> GetETotal(i32 Start, i32 End) const;

    void Tick();
private:
    struct ConservedStates
    {
        std::vector<f32> rho = {  };
        std::vector<f32> rho_u = {  };
        std::vector<f32> rho_v = {  };
        std::vector<f32> e_total = {  };
    };

    struct ConservedState
    {
        f32 rho = 0.0f;
        f32 rho_u = 0.0f;
        f32 rho_v = 0.0f;
        f32 e_total = 0.0f;
    };

    struct DerivedState
    {
        f32 u = 0.0f;
        f32 v = 0.0f;
        f32 P = 0.0f;
        f32 c = 0.0f;
    };

    struct Flux
    {
        f32 Mass = 0.0f;
        f32 u_momentum = 0.0f;
        f32 v_momentum = 0.0f;
        f32 Energy = 0.0f;
    };

    struct Fluxes
    {
        std::vector<f32> Mass = {  };
        std::vector<f32> u_momentum = {  };
        std::vector<f32> v_momentum = {  };
        std::vector<f32> Energy = {  };
    };

    const Mesh* SceneMesh = nullptr;

    f32 dx = 0.0f;
    f32 dy = 0.0f;
    f32 dt = 0.0f;
    f32 R = 0.0f;
    f32 gamma = 0.0f;

    ConservedStates State = {  };
    ConservedStates TempState = {  };
    Fluxes DeltaFluxes = {  };

    ConservedState GetConservedState(i32 CellIdx);
    ConservedState GetSlipWallGhostState(const ConservedState& State, const vec2& Normal);
    Flux GetDeltaFlux(const ConservedState& Left, const ConservedState& Right, const vec2& Normal);

    f32 Rusanov(f32 FluxLeft, f32 FluxRight, f32 alpha, f32 ConservedLeft, f32 ConservedRight);
    
};

