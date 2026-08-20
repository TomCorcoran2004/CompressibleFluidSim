#pragma once
#include <glm/glm.hpp>
#include <iostream>

class RiemannSolver
{
public:
    struct W
    {
        f32 rho = 0.0f;
        f32 u = 0.0f;
        f32 p = 0.0f;
    };
    
    struct Config
    {
        W Left; 
        W Right;
        f32 gamma;
    };

    RiemannSolver(const Config& Config);
    W Sample(f32 x, f32 t);

    static bool UnitTests();
    
private:
    const W Left;
    const W Right;
    
    f32 gamma;
    f32 p_star;
    f32 u_star;
    f32 rho_star_L;
    f32 rho_star_R;

    f32 GetShockSpeed(f32 pk, f32 a);

    f32 EvaluatePressure(f32 p, const W& State);
    f32 EvaluatePressureDerivative(f32 p, const W& State);
    f32 EvaluatePressureResidual(f32 p);
    f32 EvaluatePressureResidualDerivative(f32 p);
    f32 NewtonRaphson(f32 p);
    f32 SolvePressureStar();
    f32 SolveRhoStar(const W& State);
    W EvaluateLeftFan(f32 xi);
    W EvaluateRightFan(f32 xi);


    static bool UnitTest1();
    static bool UnitTest2();
    static bool UnitTest3();
    static bool UnitTest4();
    static bool UnitTest5();
};



