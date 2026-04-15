#include "Sim.h"
#include <vector>
#include <GLM/glm.hpp>
#include <utility>

namespace Sim
{
    constexpr i32 GhostCellIndex = -1;
    

    namespace Mesh
    {
        using BoundaryTypes = u8;
        constexpr BoundaryTypes SolidSlip = 0b00000001;
        constexpr BoundaryTypes SolidNoSlip = 0b00000010;
        constexpr BoundaryTypes SupersonicInflow = 0b00000100;
        constexpr BoundaryTypes SubSonicInflow = 0b00001000;
        constexpr BoundaryTypes SupersonicOutflow = 0b00010000;
        constexpr BoundaryTypes SubsonicOutFlow = 0b00100000;
        constexpr BoundaryTypes Periodic = 0b01000000;
        constexpr BoundaryTypes NoBoundry = 0b10000000;

        using Direction = u8;
        constexpr Direction Vertical = 0;
        constexpr Direction Horizontal = 1;

        using CellType = u8;
        constexpr CellType Solid = 0;
        constexpr CellType Fluid = 1;

        //metadata
        ivec2 HorizontalFacesSize = ivec2(0, 0);
        i32 HorizontalFacesSizeFlat = 0;
        ivec2 VerticalFacesSize = ivec2(0, 0);
        i32 VerticalFacesSizeFlat = 0;
        i32 TotalFacesSize = 0;
        ivec2 GridSize = ivec2(0, 0);
        i32 GridSizeFlat = 0;
        i32 VerticalFacesStartingIndex = 0;

        //faces
        std::vector<i32> LeftCells;
        std::vector<i32> RightCells;
        std::vector<BoundaryTypes> FaceTypes;
        std::vector<Direction> Directions;

        //cells
        std::vector<u8> CellTypes;

        void SetupCells()
        {
            for (i32 x = 0; x < GridSize.x; ++x)
            {
                for (i32 y = 0; y < GridSize.y; ++y)
                {
                    i32 Index = y * GridSize.x + x;

                    if (x == 0 || y == 0 || x == GridSize.x - 1 || y == GridSize.y - 1)
                    {
                        CellTypes[Index] = Solid;
                    }
                    else
                    {
                        CellTypes[Index] = Fluid;
                    }
                }
            }
        }

        void SetupFaces()
        {
            for (i32 i = 0; i < HorizontalFacesSizeFlat; ++i)
            {
                ivec2 Position;
                Position.x = i % HorizontalFacesSize.x;
                Position.y = i / HorizontalFacesSize.x;

                i32 LeftCellIndex = Position.y * GridSize.x + Position.x;
                i32 RightCellIndex = LeftCellIndex + GridSize.x;

                LeftCells[i] = LeftCellIndex;
                RightCells[i] = RightCellIndex;
                Directions[i] = Horizontal;

                if (CellTypes[LeftCellIndex] == Solid || CellTypes[RightCellIndex] == Solid)
                    FaceTypes[i] = NoBoundry;
                else
                    FaceTypes[i] = NoBoundry;
            }

            for (i32 i = VerticalFacesStartingIndex; i < TotalFacesSize; ++i)
            {
                ivec2 Position;
                Position.x = (i - VerticalFacesStartingIndex) % VerticalFacesSize.x;
                Position.y = (i - VerticalFacesStartingIndex) / VerticalFacesSize.x;

                i32 LeftCellIndex = Position.y * GridSize.x + Position.x;
                i32 RightCellIndex = LeftCellIndex + 1;

                LeftCells[i] = LeftCellIndex;
                RightCells[i] = RightCellIndex;
                Directions[i] = Vertical;

                if (CellTypes[LeftCellIndex] == Solid || CellTypes[RightCellIndex] == Solid)
                    FaceTypes[i] = SolidSlip;
                else
                    FaceTypes[i] = NoBoundry;
            }
        }

        void Init(ivec2 _GridSize)
        {
            GridSize = _GridSize;

            HorizontalFacesSize = ivec2(GridSize.x, GridSize.y - 1);
            HorizontalFacesSizeFlat = HorizontalFacesSize.x * HorizontalFacesSize.y;

            VerticalFacesSize = ivec2(GridSize.x - 1, GridSize.y);
            VerticalFacesSizeFlat = VerticalFacesSize.x * VerticalFacesSize.y;

            TotalFacesSize = HorizontalFacesSizeFlat + VerticalFacesSizeFlat;
            VerticalFacesStartingIndex = HorizontalFacesSizeFlat;

            GridSizeFlat = GridSize.x * GridSize.y;
                
            LeftCells.resize(TotalFacesSize);
            RightCells.resize(TotalFacesSize);
            FaceTypes.resize(TotalFacesSize);
            Directions.resize(TotalFacesSize);

            CellTypes.resize(GridSizeFlat);

            SetupCells();
            SetupFaces();
        }
    };

    
    bool Init()
    {
        Mesh::Init(Config::GridSize);
        
        ivec2 GridSize = Mesh::GridSize;
        i32 GridSizeFlat = Mesh::GridSizeFlat;

        rho.resize(GridSizeFlat);
        rho_u.resize(GridSizeFlat);
        rho_v.resize(GridSizeFlat);
        e_total.resize(GridSizeFlat);

        rho_temp.resize(GridSizeFlat);
        rho_u_temp.resize(GridSizeFlat);
        rho_v_temp.resize(GridSizeFlat);
        e_total_temp.resize(GridSizeFlat);

        //initialise starting state
        std::fill(rho.begin(), rho.end(), Config::InitialState::rho);
        std::fill(rho_u.begin(), rho_u.end(), Config::InitialState::rho * Config::InitialState::u);
        std::fill(rho_v.begin(), rho_v.end(), Config::InitialState::rho * Config::InitialState::v);
        std::fill(e_total.begin(), e_total.end(), Config::InitialState::P / (Config::gamma - 1.0f) + 0.5f * Config::InitialState::rho * (Config::InitialState::u * Config::InitialState::u + Config::InitialState::v * Config::InitialState::v));



        std::fill(rho_temp.begin(), rho_temp.end(), 0.0f);
        std::fill(rho_u_temp.begin(), rho_u_temp.end(), 0.0f);
        std::fill(rho_v_temp.begin(), rho_v_temp.end(), 0.0f);
        std::fill(e_total_temp.begin(), e_total_temp.end(), 0.0f);

        for (int y = 0; y < Mesh::GridSize.y; y++)
        {
            for (int x = 0; x < Mesh::GridSize.x; x++)
            {
                int idx = y * Mesh::GridSize.x + x;

                float rho0, u0, v0, P0;

                if (x < Mesh::GridSize.x / 2)
                {
                    rho0 = 1.0f;
                    u0 = 0.0f;
                    v0 = 0.0f;
                    P0 = 1.0f;
                }
                else
                {
                    rho0 = 0.125f;
                    u0 = 0.0f;
                    v0 = 0.0f;
                    P0 = 0.1f;
                }

                rho[idx] = rho0;
                rho_u[idx] = rho0 * u0;
                rho_v[idx] = rho0 * v0;
                e_total[idx] = P0 / (Config::gamma - 1.0f) + 0.5f * rho0 * (u0 * u0 + v0 * v0);
            }
        }

        return true;
    }

    vec4 CreateSlipState(const vec4& Other, u8 Direction)
    {
        //Compute Primitives From Existing State
        f32 u = Other.y / Other.x;// u = rho_u / u
        f32 v = Other.z / Other.x;// v = rho_v / v
        f32 P = (Config::gamma - 1.0f) * (Other.w - 0.5f * Other.x * (u * u + v * v)); // (gamma - 1) * (e_total * 1/2 * rho * (u^2 + v^2)

        //flip normal velocity to enforce no mass transfer
        if (Direction == Mesh::Horizontal) u = -u;
        else v = -v;

        //create ghost state
        vec4 GhostState;
        GhostState.x = Other.x;
        GhostState.y = u * GhostState.x;
        GhostState.z = v * GhostState.x;
        GhostState.w = P / (Config::gamma - 1.0f) + 0.5f * GhostState.x * (u * u + v * v);

        return GhostState;
    }

    bool GetConservedStatesGodunov(i32 FaceIndex, vec4& LeftState, vec4& RightState)
    {
        u8 FaceType = Mesh::FaceTypes[FaceIndex];
        i32 LeftCellIndex = Mesh::LeftCells[FaceIndex];
        i32 RightCellIndex = Mesh::RightCells[FaceIndex];

        u8 LeftCellType = Mesh::CellTypes[LeftCellIndex];
        u8 RightCellType = Mesh::CellTypes[RightCellIndex];

        u8 Direction = Mesh::Directions[FaceIndex];

        if (LeftCellType == Mesh::Solid && RightCellType == Mesh::Solid)
            return false;

        switch (FaceType)
        {
            case(Mesh::NoBoundry):
            {
                LeftState.x = rho[LeftCellIndex];
                LeftState.y = rho_u[LeftCellIndex];
                LeftState.z = rho_v[LeftCellIndex];
                LeftState.w = e_total[LeftCellIndex];

                RightState.x = rho[RightCellIndex];
                RightState.y = rho_u[RightCellIndex];
                RightState.z = rho_v[RightCellIndex];
                RightState.w = e_total[RightCellIndex];

                return true;
            }
            case(Mesh::SolidSlip):
            {
                if (Mesh::CellTypes[LeftCellIndex] == Mesh::Solid)
                {
                    RightState.x = rho[RightCellIndex];
                    RightState.y = rho_u[RightCellIndex];
                    RightState.z = rho_v[RightCellIndex];
                    RightState.w = e_total[RightCellIndex];
                    
                    LeftState = CreateSlipState(RightState, Direction);
                }
                else
                {
                    LeftState.x = rho[RightCellIndex];
                    LeftState.y = rho_u[RightCellIndex];
                    LeftState.z = rho_v[RightCellIndex];
                    LeftState.w = e_total[RightCellIndex];

                    RightState = CreateSlipState(LeftState, Direction);
                }

                return true;
            }
            default: return false;

        }
    }

    vec4 RusanovSolver(const vec4& Left, const vec4& Right, u8 Direction)
    {
        f32 rho_L = glm::max(Left.x, 1e-6f);
        f32 rho_u_L = Left.y;
        f32 rho_v_L = Left.z;
        f32 e_total_L  = Left.w;
        
        f32 u_L = rho_u_L / rho_L;
        f32 v_L = rho_v_L / rho_L;
        f32 P_L = (Config::gamma - 1.0f) * (e_total_L - 0.5f * rho_L * (u_L * u_L + v_L * v_L));
        P_L = glm::max(P_L, 1e-6f);

        vec4 FL;
        FL.x = (Direction == Mesh::Horizontal) ? rho_u_L : rho_v_L;
        FL.y = (Direction == Mesh::Horizontal) ? rho_L * u_L * u_L + P_L : rho_L * v_L * u_L;
        FL.z = (Direction == Mesh::Horizontal) ? rho_L * u_L * v_L : rho_L * v_L * v_L + P_L;
        FL.w = (Direction == Mesh::Horizontal) ? (e_total_L + P_L) * u_L : (e_total_L + P_L) * v_L;

        f32 rho_R = glm::max(Right.x, 1e-6f);
        f32 rho_u_R = Right.y;
        f32 rho_v_R = Right.z;
        f32 e_total_R = Right.w;

        f32 u_R = rho_u_R / rho_R;
        f32 v_R = rho_v_R / rho_R;
        f32 P_R = (Config::gamma - 1.0f) * (e_total_R - 0.5f * rho_R * (u_R * u_R + v_R * v_R));
        P_R = glm::max(P_R, 1e-6f);

        vec4 FR;
        FR.x = (Direction == Mesh::Horizontal) ? rho_u_R : rho_v_R;
        FR.y = (Direction == Mesh::Horizontal) ? rho_R * u_R * u_R + P_R : rho_R * v_R * u_R;
        FR.z = (Direction == Mesh::Horizontal) ? rho_R * u_R * v_R : rho_R * v_R * v_R + P_R;
        FR.w = (Direction == Mesh::Horizontal) ? (e_total_R + P_R) * u_R : (e_total_R + P_R) * v_R;

        f32 C_L = glm::sqrt(Config::gamma * P_L / rho_L);
        f32 C_R = glm::sqrt(Config::gamma * P_R / rho_R);

        f32 alpha = 0.0f;
        if (Direction == Mesh::Horizontal) alpha = glm::max(glm::abs(u_L) + C_L, glm::abs(u_R) + C_R);
        else alpha = glm::max(glm::abs(v_L) + C_L, glm::abs(v_R) + C_R);

        vec4 DeltaFaceFlux;
        DeltaFaceFlux.x = (FL.x + FR.x) * 0.5f - 0.5f * alpha * (rho_R - rho_L);
        DeltaFaceFlux.y = (FL.y + FR.y) * 0.5f - 0.5f * alpha * (rho_u_R - rho_u_L);
        DeltaFaceFlux.z = (FL.z + FR.z) * 0.5f - 0.5f * alpha * (rho_v_R - rho_v_L);
        DeltaFaceFlux.w = (FL.w + FR.w) * 0.5f - 0.5f * alpha * (e_total_R - e_total_L);

        return DeltaFaceFlux;
    }

    void Tick()
    {
        rho_temp = rho;
        rho_u_temp = rho_u;
        rho_v_temp = rho_v;
        e_total_temp = e_total;

        for (i32 i = 0; i < Mesh::TotalFacesSize; ++i)
        {
            vec4 LeftState = vec4(0.0f, 0.0f, 0.0f, 0.0f);
            vec4 RightState = vec4(0.0f, 0.0f, 0.0f, 0.0f);
            
            u8 Direction = Mesh::Directions[i];
            i32 LeftCellIndex = Mesh::LeftCells[i];
            i32 RightCellIndex = Mesh::RightCells[i];

            if (GetConservedStatesGodunov(i, LeftState, RightState) == false) continue;
           
            vec4 DeltaFlux = RusanovSolver(LeftState, RightState, Direction);

            if (Mesh::CellTypes[LeftCellIndex] == Mesh::Fluid)
            {
                rho_temp[LeftCellIndex] -= DeltaFlux.x * Config::dt;
                rho_u_temp[LeftCellIndex] -= DeltaFlux.y * Config::dt;
                rho_v_temp[LeftCellIndex] -= DeltaFlux.z * Config::dt;
                e_total_temp[LeftCellIndex] -= DeltaFlux.w * Config::dt;
            }

            if (Mesh::CellTypes[RightCellIndex] == Mesh::Fluid)
            {
                rho_temp[RightCellIndex] += DeltaFlux.x * Config::dt;
                rho_u_temp[RightCellIndex] += DeltaFlux.y * Config::dt;
                rho_v_temp[RightCellIndex] += DeltaFlux.z * Config::dt;
                e_total_temp[RightCellIndex] += DeltaFlux.w * Config::dt;
            }
        }

        std::swap(rho, rho_temp);
        std::swap(rho_u, rho_u_temp);
        std::swap(rho_v, rho_v_temp);
        std::swap(e_total, e_total_temp);
    }

    void Destroy() {};
}