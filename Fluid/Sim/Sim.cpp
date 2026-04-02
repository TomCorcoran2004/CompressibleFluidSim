#include "Sim.h"
#include <vector>
#include <GLM/glm.hpp>

namespace Sim
{
    struct Flux
    {
        std::vector<f32> Mass = {  };
        std::vector<f32> XMomentum = {  };
        std::vector<f32> YMomentum = {  };
        std::vector<f32> Energy = {  };

        void resize(i32 NewSize)
        {
            Mass.resize(NewSize);
            XMomentum.resize(NewSize);
            YMomentum.resize(NewSize);
            Energy.resize(NewSize);
        }

        void fill(f32 Val)
        {
            std::fill(Mass.begin(), Mass.end(), Val);
            std::fill(XMomentum.begin(), XMomentum.end(), Val);
            std::fill(YMomentum.begin(), YMomentum.end(), Val);
            std::fill(Energy.begin(), Energy.end(), Val);
        }
    };
    
    namespace Config
    {
        constexpr f32 dt = 1.0f / 100.0f;
        constexpr f32 dx = 1.0f;
        constexpr f32 R = 1.0f;
        constexpr f32 gamma = 1.0f;

        constexpr ivec2 GridSize = ivec2(100, 100);
    }

    constexpr i32 GridSizeFlat = Config::GridSize.x * Config::GridSize.y;

    //Conserverd State
    std::vector<f32> rho_u;
    std::vector<f32> rho_v;
    std::vector<f32> rho;
    std::vector<f32> e_total;

    //Primitve Vars
    std::vector<f32> u;
    std::vector<f32> v;
    std::vector<f32> P;
    std::vector<f32> T;

    Flux x_flux;
    Flux y_flux;

    bool Init()
    {
        i32 GridSizeFlat = Config::GridSize.x * Config::GridSize.y;

        rho_u.resize(GridSizeFlat);
        rho_v.resize(GridSizeFlat);
        u.resize(GridSizeFlat);
        v.resize(GridSizeFlat);
        e_total.resize(GridSizeFlat);
        rho.resize(GridSizeFlat);
        P.resize(GridSizeFlat);
        T.resize(GridSizeFlat);
        x_flux.resize(GridSizeFlat);
        y_flux.resize(GridSizeFlat);

        std::fill(rho_u.begin(), rho_u.end(), 0.0f);
        std::fill(rho_v.begin(), rho_v.end(), 0.0f);
        std::fill(u.begin(), u.end(), 0.0f);
        std::fill(v.begin(), v.end(), 0.0f);
        std::fill(e_total.begin(), e_total.end(), 0.0f);
        std::fill(rho.begin(), rho.end(), 0.0f);
        std::fill(P.begin(), P.end(), 0.0f);
        std::fill(T.begin(), T.end(), 0.0f);
        x_flux.fill(0.0f);
        y_flux.fill(0.0f);

        return true;
    }

    void ComputeVelocity()
    {
        for (i32 i = 0; i < GridSizeFlat; ++i)
        {
            u[i] = rho_u[i] / rho[i];
            v[i] = rho_v[i] / rho[i];
        }
    }

    void ComputePressure()
    {
        for (i32 i = 0; i < GridSizeFlat; ++i)
        {
            //P = (Gamma - 1) * InternalEnergy
            //P = (Gamma - 1) * (TotalEnergy - 1/2 * rho * v^2)
            P[i] = (Config::gamma - 1.0f) * (e_total[i] - 0.5f * rho[i] * (u[i] * u[i] + v[i] * v[i]));
        }
    }

    void ComputeTemperature()
    {
        for (i32 i = 0; i < GridSizeFlat; ++i)
        {
            //Temperature = Pressure / (rho * R)
            T[i] = P[i] / (rho[i] * Config::R);
        }
    }

    void ComputeFlux()
    {
        for (i32 i = 0; i < GridSizeFlat; ++i)
        {
            x_flux.Mass[i] = rho_u[i];
            x_flux.XMomentum[i] = rho[i] * u[i] * u[i] + P[i];
            x_flux.YMomentum[i] = rho[i] * u[i] * v[i];
            x_flux.Energy[i] = (e_total[i] + P[i]) * u[i];

            y_flux.Mass[i] = rho_v[i];
            y_flux.XMomentum[i] = rho[i] * v[i] * u[i];
            y_flux.YMomentum[i] = rho[i] * v[i] * v[i] + P[i];
            y_flux.Energy[i] = (e_total[i] + P[i]) * v[i];
        }
    }

    void Tick()
    {
        ComputeVelocity();
        ComputePressure();
        ComputeTemperature();
        ComputeFlux();
        
    }
}