#include "CompressibleFluidSim/fluid/solver/riemann_solver.h"
#include "CompressibleFluidSim/fluid/utils/types.h"
#include <iostream>

bool epsilon_not_equal(f32 a, f32 b, f32 tol)
{
    return std::abs(a - b) > tol;
}

bool riemann_solver::unit_tests()
{
    if (unit_test1() == false) return false;
    if (unit_test2() == false) return false;
    if (unit_test3() == false) return false;
    if (unit_test4() == false) return false;
    if (unit_test5() == false) return false;

    return true;
}

bool riemann_solver::unit_test1()
{
    constexpr f32 tolerance = 0.5e-5f;

    riemann_solver::w left = {
            .rho = 1.0f,
            .u = 0.0f,
            .p = 1.0f
    };

    riemann_solver::w right = {
        .rho = 0.125f,
        .u = 0.0f,
        .p = 0.1f
    };

    riemann_solver::config riemann_config = {
        .left = left,
        .right = right,
        .gamma = 1.4f
    };

    riemann_solver solver = { riemann_config };

    if (epsilon_not_equal(solver.p_star, 0.30313f, tolerance))
    {
        std::cout << "Unit Test 1 Failed: P* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.u_star, 0.92745f, tolerance))
    {
        std::cout << "Unit Test 1 Failed: u* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_l, 0.42632f, tolerance))
    {
        std::cout << "Unit Test 1 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_r, 0.26557f, tolerance))
    {
        std::cout << "Unit Test 1 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 1 Passed\n";
    return true;
}

bool riemann_solver::unit_test2()
{
    constexpr f32 tolerance = 0.5e-5f;

    riemann_solver::w left = {
            .rho = 1.0f,
            .u = -2.0f,
            .p = 0.4f
    };

    riemann_solver::w right = {
        .rho = 1.0f,
        .u = 2.0f,
        .p = 0.4f
    };

    riemann_solver::config riemann_config = {
        .left = left,
        .right = right,
        .gamma = 1.4f
    };

    riemann_solver solver = { riemann_config };

    if (epsilon_not_equal(solver.p_star, 0.00189f, tolerance))
    {
        std::cout << "Unit Test 2 Failed: P* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.u_star, 0.0f, tolerance))
    {
        std::cout << "Unit Test 2 Failed: u* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_l, 0.02185f, tolerance))
    {
        std::cout << "Unit Test 2 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_r, 0.02185f, tolerance))
    {
        std::cout << "Unit Test 2 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 2 Passed\n";
    return true;
}

bool riemann_solver::unit_test3()
{
    constexpr f32 tolerance = 0.5e-3f;

    riemann_solver::w left = {
            .rho = 1.0f,
            .u = 0.0f,
            .p = 1000.0f
    };

    riemann_solver::w right = {
        .rho = 1.0f,
        .u = 0.0f,
        .p = 0.01f
    };

    riemann_solver::config riemann_config = {
        .left = left,
        .right = right,
        .gamma = 1.4f
    };

    riemann_solver solver = { riemann_config };

    if (epsilon_not_equal(solver.p_star, 460.894f, tolerance))
    {
        std::cout << "Unit Test 3 Failed: P* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.u_star, 19.5975f, tolerance))
    {
        std::cout << "Unit Test 3 Failed: u* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_l, 0.57506f, tolerance))
    {
        std::cout << "Unit Test 3 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_r, 5.99924f, tolerance))
    {
        std::cout << "Unit Test 3 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 3 Passed\n";
    return true;
}

bool riemann_solver::unit_test4()
{
    constexpr f32 tolerance = 0.5e-4f;

    riemann_solver::w left = {
            .rho = 1.0f,
            .u = 0.0f,
            .p = 0.01f
    };

    riemann_solver::w right = {
        .rho = 1.0f,
        .u = 0.0f,
        .p = 100.0f
    };

    riemann_solver::config riemann_config = {
        .left = left,
        .right = right,
        .gamma = 1.4f
    };

    riemann_solver solver = { riemann_config };

    if (epsilon_not_equal(solver.p_star, 46.0950f, tolerance))
    {
        std::cout << "Unit Test 4 Failed: P* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.u_star, -6.19633f, tolerance))
    {
        std::cout << "Unit Test 4 Failed: u* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_l, 5.99242f, tolerance))
    {
        std::cout << "Unit Test 4 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_r, 0.57511f, tolerance))
    {
        std::cout << "Unit Test 4 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 4 Passed\n";
    return true;
}

bool riemann_solver::unit_test5()
{
    constexpr f32 tolerance = 1.0e-2f;

    riemann_solver::w left = {
            .rho = 5.99924f,
            .u = 19.5975f,
            .p = 460.894f
    };

    riemann_solver::w right = {
        .rho = 5.99242f,
        .u = -6.19633f,
        .p = 46.0950f
    };

    riemann_solver::config riemann_config = {
        .left = left,
        .right = right,
        .gamma = 1.4f
    };

    riemann_solver solver = { riemann_config };

    if (epsilon_not_equal(solver.p_star, 1691.64f, tolerance))
    {
        std::cout << "Unit Test 5 Failed: P* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.u_star, 8.68975f, tolerance))
    {
        std::cout << "Unit Test 5 Failed: u* Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_l, 14.2823f, tolerance))
    {
        std::cout << "Unit Test 5 Failed: Rho*L Incorrect\n";
        return false;
    }

    if (epsilon_not_equal(solver.rho_star_r, 31.0426f, tolerance))
    {
        std::cout << "Unit Test 5 Failed: Rho*R Incorrect\n";
        return false;
    }

    std::cout << "Unit Test 5 Passed\n";
    return true;
}