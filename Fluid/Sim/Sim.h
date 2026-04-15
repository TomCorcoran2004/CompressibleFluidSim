#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace Sim
{
    bool Init();
    void Tick();
    void Destroy();

    namespace Config
    {
        constexpr f32 dt = 1.0f / 1000.0f;
        constexpr f32 dx = 1.0f;
        constexpr f32 R = 1.0f;
        constexpr f32 gamma = 1.4f;

        namespace InitialState
        {
            constexpr f32 P = 101325.0f;
            constexpr f32 u = 0.0f;
            constexpr f32 v = 0.0f;
            constexpr f32 rho = 1.0f;
        }

        constexpr ivec2 GridSize = ivec2(40, 40);
    }

    //Conserverd State
    inline std::vector<f32> rho;
    inline std::vector<f32> rho_u;
    inline std::vector<f32> rho_v;
    inline std::vector<f32> e_total;

    inline std::vector<f32> rho_temp;
    inline std::vector<f32> rho_u_temp;
    inline std::vector<f32> rho_v_temp;
    inline std::vector<f32> e_total_temp;
}

