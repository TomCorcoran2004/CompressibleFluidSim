#pragma once
#include "types.h"


class riemann_solver
{
public:
    struct w
    {
        f32 rho = 0.0f;
        f32 u = 0.0f;
        f32 p = 0.0f;
    };

    struct config
    {
        w left;
        w right;
        f32 gamma;
    };

    riemann_solver(const config& config);
    w sample(f32 x, f32 t);

    static bool unit_tests();

private:
    const w left;
    const w right;

    f32 gamma;
    f32 p_star;
    f32 u_star;
    f32 rho_star_l;
    f32 rho_star_r;

    f32 get_shock_speed(f32 pk, f32 a);

    f32 evaluate_pressure(f32 p, const w& state);
    f32 evaluate_pressure_derivative(f32 p, const w& state);
    f32 evaluate_pressure_residual(f32 p);
    f32 evaluate_pressure_residual_derivative(f32 p);
    f32 newton_raphson(f32 p);
    f32 solve_pressure_star();
    f32 solve_rho_star(const w& state);
    w evaluate_left_fan(f32 xi);
    w evaluate_right_fan(f32 xi);


    static bool unit_test1();
    static bool unit_test2();
    static bool unit_test3();
    static bool unit_test4();
    static bool unit_test5();
};