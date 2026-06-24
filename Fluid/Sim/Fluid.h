#pragma once
#include "glm/glm.hpp"
#include <span>
#include <vector>
#include "Mesh.h"

class Fluid
{
public:
    struct Config
    {
        f32 dt = 0.001f;
        f32 R = 1.0f;
        f32 gamma = 1.4f;
    };

    Fluid();
    Fluid(const Config& Config, const Mesh& Mesh);

    void Tick();

    void SetRho(i32 Index, f32 Val);
    void SetU(i32 Index, f32 Val);
    void SetV(i32 Index, f32 Val);
    void SetP(i32 Index, f32 Val);

    std::span<const f32> GetRho() const;
    std::span<const f32> GetRhoU() const;
    std::span<const f32> GetRhoV() const;
    std::span<const f32> GetETotal() const;
private:
    const Mesh* SceneMesh;

    f32 dt;
    f32 dx;
    f32 dy;
    f32 R;
    f32 gamma;

    std::vector<i32> LeftState;
    std::vector<i32> RightState;

    std::vector<f32> rho;
    std::vector<f32> rho_u;
    std::vector<f32> rho_v;
    std::vector<f32> e_total;

    std::vector<f32> rho_temp;
    std::vector<f32> rho_u_temp;
    std::vector<f32> rho_v_temp;
    std::vector<f32> e_total_temp;

    std::vector<vec4> DeltaFlux;

    vec4 CreateSolidSlipState(i32 Other, const vec2& Normal);
    vec4 RusanovSolver(i32 LeftStateIdx, i32 RightStateIdx, const vec2& Normal);
    i32 AddGhostState();
};

