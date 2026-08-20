#include "Riemann.h"
#include <glm/gtc/epsilon.hpp>
#include <iostream>

bool RiemannSolver::UnitTests()
{
    if (UnitTest1() == false) return false;
    if (UnitTest2() == false) return false;
    if (UnitTest3() == false) return false;
    if (UnitTest4() == false) return false;
    if (UnitTest5() == false) return false;

    return true;
}

bool RiemannSolver::UnitTest1()
{
    constexpr f32 Tolerance = 0.5e-5f;

    RiemannSolver::W Left = {
            .rho = 1.0f,
            .u = 0.0f,
            .p = 1.0f
    };

    RiemannSolver::W Right = {
        .rho = 0.125f,
        .u = 0.0f,
        .p = 0.1f
    };

    RiemannSolver::Config RiemannConfig = {
        .Left = Left,
        .Right = Right,
        .gamma = 1.4f
    };

    RiemannSolver Solver = { RiemannConfig };

    if (glm::epsilonNotEqual(Solver.p_star, 0.30313f, Tolerance))
    {
        std::cout << "Unit Test 1 Failed: P* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.u_star, 0.92745f, Tolerance))
    {
        std::cout << "Unit Test 1 Failed: u* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_L, 0.42632f, Tolerance))
    {
        std::cout << "Unit Test 1 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_R, 0.26557f, Tolerance))
    {
        std::cout << "Unit Test 1 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 1 Passed\n";
    return true;
}

bool RiemannSolver::UnitTest2()
{
    constexpr f32 Tolerance = 0.5e-5f;

    RiemannSolver::W Left = {
            .rho = 1.0f,
            .u = -2.0f,
            .p = 0.4f
    };

    RiemannSolver::W Right = {
        .rho = 1.0f,
        .u = 2.0f,
        .p = 0.4f
    };

    RiemannSolver::Config RiemannConfig = {
        .Left = Left,
        .Right = Right,
        .gamma = 1.4f
    };

    RiemannSolver Solver = { RiemannConfig };

    if (glm::epsilonNotEqual(Solver.p_star, 0.00189f, Tolerance))
    {
        std::cout << "Unit Test 2 Failed: P* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.u_star, 0.0f, Tolerance))
    {
        std::cout << "Unit Test 2 Failed: u* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_L, 0.02185f, Tolerance))
    {
        std::cout << "Unit Test 2 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_R, 0.02185f, Tolerance))
    {
        std::cout << "Unit Test 2 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 2 Passed\n";
    return true;
}

bool RiemannSolver::UnitTest3()
{
    constexpr f32 Tolerance = 0.5e-3f;

    RiemannSolver::W Left = {
            .rho = 1.0f,
            .u = 0.0f,
            .p = 1000.0f
    };

    RiemannSolver::W Right = {
        .rho = 1.0f,
        .u = 0.0f,
        .p = 0.01f
    };

    RiemannSolver::Config RiemannConfig = {
        .Left = Left,
        .Right = Right,
        .gamma = 1.4f
    };

    RiemannSolver Solver = { RiemannConfig };

    if (glm::epsilonNotEqual(Solver.p_star, 460.894f, Tolerance))
    {
        std::cout << "Unit Test 3 Failed: P* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.u_star, 19.5975f, Tolerance))
    {
        std::cout << "Unit Test 3 Failed: u* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_L, 0.57506f, Tolerance))
    {
        std::cout << "Unit Test 3 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_R, 5.99924f, Tolerance))
    {
        std::cout << "Unit Test 3 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 3 Passed\n";
    return true;
}

bool RiemannSolver::UnitTest4()
{
    constexpr f32 Tolerance = 0.5e-4f;

    RiemannSolver::W Left = {
            .rho = 1.0f,
            .u = 0.0f,
            .p = 0.01f
    };

    RiemannSolver::W Right = {
        .rho = 1.0f,
        .u = 0.0f,
        .p = 100.0f
    };

    RiemannSolver::Config RiemannConfig = {
        .Left = Left,
        .Right = Right,
        .gamma = 1.4f
    };

    RiemannSolver Solver = { RiemannConfig };

    if (glm::epsilonNotEqual(Solver.p_star, 46.0950f, Tolerance))
    {
        std::cout << "Unit Test 4 Failed: P* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.u_star, -6.19633f, Tolerance))
    {
        std::cout << "Unit Test 4 Failed: u* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_L, 5.99242f, Tolerance))
    {
        std::cout << "Unit Test 4 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_R, 0.57511f, Tolerance))
    {
        std::cout << "Unit Test 4 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 4 Passed\n";
    return true;
}

bool RiemannSolver::UnitTest5()
{
    constexpr f32 Tolerance = 1.0e-2f;

    RiemannSolver::W Left = {
            .rho = 5.99924f,
            .u = 19.5975f,
            .p = 460.894f
    };

    RiemannSolver::W Right = {
        .rho = 5.99242f,
        .u = -6.19633f,
        .p = 46.0950f
    };

    RiemannSolver::Config RiemannConfig = {
        .Left = Left,
        .Right = Right,
        .gamma = 1.4f
    };

    RiemannSolver Solver = { RiemannConfig };

    if (glm::epsilonNotEqual(Solver.p_star, 1691.64f, Tolerance))
    {
        std::cout << "Unit Test 5 Failed: P* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.u_star, 8.68975f, Tolerance))
    {
        std::cout << "Unit Test 5 Failed: u* Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_L, 14.2823f, Tolerance))
    {
        std::cout << "Unit Test 5 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (glm::epsilonNotEqual(Solver.rho_star_R, 31.0426f, Tolerance))
    {
        std::cout << "Unit Test 5 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 5 Passed\n";
    return true;
}