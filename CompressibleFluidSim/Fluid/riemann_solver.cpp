#include "riemann_solver.h"
#include <iostream>

riemann_solver::riemann_solver(const config& config) : left(config.left),
right(config.right),
gamma(config.gamma)
{
    p_star = solve_pressure_star();
    u_star = 0.5f * (left.u + right.u) + 0.5 * (evaluate_pressure(p_star, right) - evaluate_pressure(p_star, left));
    rho_star_l = solve_rho_star(left);
    rho_star_r = solve_rho_star(right);
}

riemann_solver::w riemann_solver::sample(f32 x, f32 t)
{
    f32 xi = x / t;

    //left side
    if (xi <= u_star)
    {
        const f32 a = std::sqrt((gamma * left.p) / left.rho);

        //shock
        if (p_star > left.p)
        {

            const f32 relative_shock_speed = get_shock_speed(left.p, a);
            f32 s = left.u - relative_shock_speed;

            if (xi < s)
            {
                return left;
            }
            else
            {
                return w{
                    .rho = rho_star_l,
                    .u = u_star,
                    .p = p_star
                };
            }
        }

        else //rarefaction
        {
            const f32 a_star = a * std::pow(p_star / left.p, (gamma - 1.0f) / (2.0f * gamma));

            f32 shl = left.u - a;
            f32 stl = u_star - a_star;

            if (xi < shl)
            {
                return left;
            }
            else if (xi < stl)
            {
                return evaluate_left_fan(xi);
            }
            else
            {
                return w{
                    .rho = rho_star_l,
                    .u = u_star,
                    .p = p_star
                };
            }
        }
    }
    else
    {
        const f32 a = std::sqrt((gamma * right.p) / right.rho);

        //shock
        if (p_star > right.p)
        {

            const f32 relative_shock_speed = get_shock_speed(right.p, a);
            f32 s = right.u + relative_shock_speed;

            if (xi > s)
            {
                return right;
            }
            else
            {
                return w{
                    .rho = rho_star_r,
                    .u = u_star,
                    .p = p_star
                };
            }
        }
        else //rarefaction
        {
            const f32 a_star = a * std::pow(p_star / right.p, (gamma - 1.0f) / (2.0f * gamma));

            f32 shr = right.u + a;
            f32 str = u_star + a_star;

            if (xi < str)
            {
                return w{
                    .rho = rho_star_r,
                    .u = u_star,
                    .p = p_star
                };
            }
            else if (xi < shr)
            {
                return evaluate_right_fan(xi);
            }
            else
            {
                return right;
            }
        }
    }
}

f32 riemann_solver::get_shock_speed(f32 pk, f32 ak)
{
    const f32 pressure_ratio = p_star / pk;
    return ak * std::sqrt((gamma + 1.0f) / (2.0f * gamma) * pressure_ratio + (gamma - 1.0f) / (2.0f * gamma));
}

f32 riemann_solver::evaluate_pressure(f32 p, const w& state)
{
    if (p > state.p)
    {
        const f32 a_denominator = (gamma + 1.0f) * state.rho;
        const f32 a_numerator = 2.0f;
        const f32 a = a_numerator / a_denominator;

        const f32 b_denominator = gamma + 1.0f;
        const f32 b_numerator = (gamma - 1.0f) * state.p;
        const f32 b = b_numerator / b_denominator;

        return (p - state.p) * std::sqrt(a / (p + b));
    }
    else
    {
        const f32 a = std::sqrt((gamma * state.p) / state.rho);
        const f32 coef = (2.0f * a) / (gamma - 1.0f);
        const f32 exponent = (gamma - 1.0f) / (2.0f * gamma);

        return coef * (std::pow(p / state.p, exponent) - 1.0f);
    }
}

f32 riemann_solver::evaluate_pressure_derivative(f32 p, const w& state)
{
    if (p > state.p)
    {
        const f32 a_denominator = (gamma + 1.0f) * state.rho;
        const f32 a_numerator = 2.0f;
        const f32 a = a_numerator / a_denominator;

        const f32 b_denominator = gamma + 1.0f;
        const f32 b_numerator = (gamma - 1.0f) * state.p;
        const f32 b = b_numerator / b_denominator;

        const f32 coef = 1.0f - (p - state.p) / (2.0f * (b + p));

        return coef * std::sqrt(a / (b + p));
    }
    else
    {
        const f32 a = std::sqrt((gamma * state.p) / state.rho);
        const f32 coef = 1.0f / (state.rho * a);
        const f32 exponent = (-1.0f * (gamma + 1.0f)) / (2.0f * gamma);

        return coef * std::pow(p / state.p, exponent);
    }
}

f32 riemann_solver::evaluate_pressure_residual(f32 p)
{
    return evaluate_pressure(p, left) + evaluate_pressure(p, right) + right.u - left.u;
}

f32 riemann_solver::evaluate_pressure_residual_derivative(f32 p)
{
    return evaluate_pressure_derivative(p, left) + evaluate_pressure_derivative(p, right);
}

f32 riemann_solver::newton_raphson(f32 p)
{
    return p - evaluate_pressure_residual(p) / evaluate_pressure_residual_derivative(p);
}

f32 riemann_solver::solve_pressure_star()
{
    f32 p0 = (left.p + right.p) * 0.5f;
    f32 p_n = std::max(newton_raphson(p0), 1e-8f);

    f32 dp = std::abs(p0 - p_n) / (0.5f * p0 + p_n);

    while (dp > 1e-6f)
    {
        p0 = p_n;
        p_n = std::max(newton_raphson(p0), 1e-8f);
        dp = std::abs(p0 - p_n) / (0.5f * (p0 + p_n));
    }

    return p_n;
}

f32 riemann_solver::solve_rho_star(const w& state)
{
    if (p_star > state.p)
    {
        const f32 max_shock_density_ratio = (gamma - 1.0f) / (gamma + 1.0f);
        const f32 pressure_ratio = p_star / state.p;

        const f32 numerator = state.rho * (max_shock_density_ratio + pressure_ratio);
        const f32 denominator = max_shock_density_ratio * pressure_ratio + 1.0f;

        return numerator / denominator;
    }
    else
    {
        return state.rho * std::pow(p_star / state.p, 1.0f / gamma);
    }
}

riemann_solver::w riemann_solver::evaluate_left_fan(f32 xi)
{
    const f32 a = std::sqrt((gamma * left.p) / left.rho);

    const f32 rarefaction_factor = 2.0f / (gamma + 1.0f) + (gamma - 1.0f) / ((gamma + 1.0f) * a) * (left.u - xi);

    const f32 velocity_term = a + left.u * (gamma - 1.0f) / 2.0f + xi;
    const f32 velocity_coef = 2.0f / (gamma + 1.0f);

    const f32 rho = left.rho * std::pow(rarefaction_factor, 2.0f / (gamma - 1.0f));
    const f32 u = velocity_coef * velocity_term;
    const f32 p = left.p * std::pow(rarefaction_factor, (2.0f * gamma) / (gamma - 1.0f));

    return w{
        .rho = rho,
        .u = u,
        .p = p
    };
}

riemann_solver::w riemann_solver::evaluate_right_fan(f32 xi)
{
    const f32 a = std::sqrt((gamma * right.p) / right.rho);

    const f32 rarefaction_factor = 2.0f / (gamma + 1.0f) - (gamma - 1.0f) / ((gamma + 1.0f) * a) * (right.u - xi);

    const f32 velocity_term = -a + right.u * (gamma - 1.0f) / 2.0f + xi;
    const f32 velocity_coef = 2.0f / (gamma + 1.0f);

    const f32 rho = right.rho * std::pow(rarefaction_factor, 2.0f / (gamma - 1.0f));
    const f32 u = velocity_coef * velocity_term;
    const f32 p = right.p * std::pow(rarefaction_factor, (2.0f * gamma) / (gamma - 1.0f));

    return w{
        .rho = rho,
        .u = u,
        .p = p
    };
}