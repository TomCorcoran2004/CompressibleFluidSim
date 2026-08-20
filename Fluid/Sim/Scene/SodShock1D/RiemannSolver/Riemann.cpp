#include "Riemann.h"

RiemannSolver::RiemannSolver(const Config& Config) : Left(Config.Left),
    Right(Config.Right),
    gamma(Config.gamma)
{
    p_star = SolvePressureStar();
    u_star = 0.5f * (Left.u + Right.u) + 0.5 * (EvaluatePressure(p_star, Right) - EvaluatePressure(p_star, Left));
    rho_star_L = SolveRhoStar(Left);
    rho_star_R = SolveRhoStar(Right);
}

RiemannSolver::W RiemannSolver::Sample(f32 x, f32 t)
{
    f32 xi = x / t;

    //left side
    if (xi <= u_star)
    {
        const f32 a = glm::sqrt((gamma * Left.p) / Left.rho);
        
        //shock
        if (p_star > Left.p)
        {
            
            const f32 RelativeShockSpeed = GetShockSpeed(Left.p, a);
            f32 S = Left.u - RelativeShockSpeed;

            if (xi < S)
            {
                return Left;
            }
            else
            {
                return W{
                    .rho = rho_star_L,
                    .u = u_star,
                    .p = p_star
                };
            }
        }
        else //rarefaction
        {
            const f32 a_star = a * glm::pow(p_star / Left.p, (gamma - 1.0f) / (2.0f * gamma));
            
            f32 SHL = Left.u - a;
            f32 STL = u_star - a_star;

            if (xi < SHL)
            {
                return Left;
            }
            else if (xi < STL)
            {
                return EvaluateLeftFan(xi);
            }
            else
            {
                return W{
                    .rho = rho_star_L,
                    .u = u_star,
                    .p = p_star
                };
            }
        }
    }
    else
    {
        const f32 a = glm::sqrt((gamma * Right.p) / Right.rho);

        //shock
        if (p_star > Right.p)
        {

            const f32 RelativeShockSpeed = GetShockSpeed(Right.p, a);
            f32 S = Right.u + RelativeShockSpeed;

            if (xi > S)
            {
                return Right;
            }
            else
            {
                return W{
                    .rho = rho_star_R,
                    .u = u_star,
                    .p = p_star
                };
            }
        }
        else //rarefaction
        {
            const f32 a_star = a * glm::pow(p_star / Right.p, (gamma - 1.0f) / (2.0f * gamma));

            f32 SHR = Right.u + a;
            f32 STR = u_star + a_star;

            if (xi < STR)
            {
                return W{
                    .rho = rho_star_R,
                    .u = u_star,
                    .p = p_star
                };
            }
            else if (xi < SHR)
            {
                return EvaluateRightFan(xi);
            }
            else
            {
                return Right;
            }
        }
    }
}

f32 RiemannSolver::GetShockSpeed(f32 pk, f32 ak)
{
    const f32 PressureRatio = p_star / pk;
    return ak * glm::sqrt((gamma + 1.0f) / (2.0f * gamma) * PressureRatio + (gamma - 1.0f) / (2.0f * gamma));
}

f32 RiemannSolver::EvaluatePressure(f32 p, const W& State)
{
    if (p > State.p)
    {
        const f32 ADenominator = (gamma + 1.0f) * State.rho;
        const f32 ANumerator = 2.0f;
        const f32 A = ANumerator / ADenominator;

        const f32 BDenominator = gamma + 1.0f;
        const f32 BNumerator = (gamma - 1.0f) * State.p;
        const f32 B = BNumerator / BDenominator;

        return (p - State.p) * glm::sqrt(A / (p + B));
    }
    else
    {
        const f32 a = glm::sqrt((gamma * State.p) / State.rho);
        const f32 Coef = (2.0f * a) / (gamma - 1.0f);
        const f32 Exponent = (gamma - 1.0f) / (2.0f * gamma);

        return Coef * (glm::pow(p / State.p, Exponent) - 1.0f);
    }
}

f32 RiemannSolver::EvaluatePressureDerivative(f32 p, const W& State)
{
    if (p > State.p)
    {
        const f32 ADenominator = (gamma + 1.0f) * State.rho;
        const f32 ANumerator = 2.0f;
        const f32 A = ANumerator / ADenominator;

        const f32 BDenominator = gamma + 1.0f;
        const f32 BNumerator = (gamma - 1.0f) * State.p;
        const f32 B = BNumerator / BDenominator;

        const f32 Coef = 1.0f - (p - State.p) / (2.0f * (B + p));

        return Coef * glm::sqrt(A / (B + p));
    }
    else
    {
        const f32 a = glm::sqrt((gamma * State.p) / State.rho);
        const f32 Coef = 1.0f / (State.rho * a);
        const f32 Exponent = (-1.0f * (gamma + 1.0f)) / (2.0f * gamma);

        return Coef * glm::pow(p / State.p, Exponent);
    }
}

f32 RiemannSolver::EvaluatePressureResidual(f32 p)
{
    return EvaluatePressure(p, Left) + EvaluatePressure(p, Right) + Right.u - Left.u;
}

f32 RiemannSolver::EvaluatePressureResidualDerivative(f32 p)
{
    return EvaluatePressureDerivative(p, Left) + EvaluatePressureDerivative(p, Right);
}

f32 RiemannSolver::NewtonRaphson(f32 p)
{
    return p - EvaluatePressureResidual(p) / EvaluatePressureResidualDerivative(p);
}

f32 RiemannSolver::SolvePressureStar()
{
    f32 p0 = (Left.p + Right.p) * 0.5f;
    f32 p_n = glm::max(NewtonRaphson(p0), 1e-8f);

    f32 dp = glm::abs(p0 - p_n) / (0.5f * p0 + p_n);

    while (dp > 1e-6f)
    {
        p0 = p_n;
        p_n = glm::max(NewtonRaphson(p0), 1e-8f);
        dp = glm::abs(p0 - p_n) / (0.5f * (p0 + p_n));
    }

    return p_n;
}

f32 RiemannSolver::SolveRhoStar(const W& State)
{
    if (p_star > State.p)
    {
        const f32 MaxShockDensityRatio = (gamma - 1.0f) / (gamma + 1.0f);
        const f32 PressureRatio = p_star / State.p;

        const f32 Numerator = State.rho * (MaxShockDensityRatio + PressureRatio);
        const f32 Denominator = MaxShockDensityRatio * PressureRatio + 1.0f;

        return Numerator / Denominator;
    }
    else
    {
        return State.rho * glm::pow(p_star / State.p, 1.0f / gamma);
    }
}

RiemannSolver::W RiemannSolver::EvaluateLeftFan(f32 xi)
{
    const f32 a = glm::sqrt((gamma * Left.p) / Left.rho);
    
    const f32 RarefactionFactor = 2.0f / (gamma + 1.0f) + (gamma - 1.0f) / ((gamma + 1.0f) * a) * (Left.u - xi);
    
    const f32 VelocityTerm = a + Left.u * (gamma - 1.0f) / 2.0f + xi;
    const f32 VelocityCoef = 2.0f / (gamma + 1.0f);
    
    const f32 rho = Left.rho * glm::pow(RarefactionFactor, 2.0f / (gamma - 1.0f));
    const f32 u = VelocityCoef * VelocityTerm;
    const f32 p = Left.p * glm::pow(RarefactionFactor, (2.0f * gamma) / (gamma - 1.0f));

    return W{
        .rho = rho,
        .u = u,
        .p = p
    };
}

RiemannSolver::W RiemannSolver::EvaluateRightFan(f32 xi)
{
    const f32 a = glm::sqrt((gamma * Left.p) / Left.rho);

    const f32 RarefactionFactor = 2.0f / (gamma + 1.0f) - (gamma - 1.0f) / ((gamma + 1.0f) * a) * (Left.u - xi);

    const f32 VelocityTerm = -a + Left.u * (gamma - 1.0f) / 2.0f + xi;
    const f32 VelocityCoef = 2.0f / (gamma + 1.0f);

    const f32 rho = Left.rho * glm::pow(RarefactionFactor, 2.0f / (gamma - 1.0f));
    const f32 u = VelocityCoef * VelocityTerm;
    const f32 p = Left.p * glm::pow(RarefactionFactor, (2.0f * gamma) / (gamma - 1.0f));

    return W{
        .rho = rho,
        .u = u,
        .p = p
    };
}



