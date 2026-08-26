#include "solver.h"

solver::solver(const config& config) : scene_mesh(config.scene_mesh)
{
    r = config.r;
    gamma = config.gamma;
    cfl_target = config.cfl_target;

    // Init ConservedState
    i32 num_cells = scene_mesh.get_cells_size_flat();
    conserved_states.resize(num_cells, 0.0f);
    conserved_states_temp.resize(num_cells, 0.0f);
    derived_states.resize(num_cells, 0.0f);
}

void solver::time_step()
{
    calculate_derived_states();
    calculate_time_step();

    time_elapsed += dt;

    std::copy(conserved_states.begin(), conserved_states.end(), conserved_states_temp.begin());

    calculate_face_flux_transfer();

    std::swap(conserved_states_temp, conserved_states);
}

f32 solver::ideal_gas_law_e(f32 p, f32 rho, f32 u, f32 v) const
{
    return p / (gamma - 1.0f) + 0.5f * rho * (u * u + v * v);
}

f32 solver::ideal_gas_law_p(f32 e, f32 rho, f32 u, f32 v) const
{
    return (gamma - 1.0f) * (e - 0.5f * rho * (u * u + v * v));
}

f32 solver::cfl_condition(f32 dx, f32 u, f32 a) const
{
    return (cfl_target * dx) / (std::abs(u) + a);
}

f32 solver::rusanov(f32 flux_left, f32 flux_right, f32 alpha, f32 conserved_left, f32 conserved_right) const
{
    return (flux_left + flux_right) * 0.5f - 0.5f * alpha * (conserved_right - conserved_left);
}

void solver::set_rho(i32 cell_idx, f32 val)
{
    conserved_states[conserved_fields::rho][cell_idx] = val;
    derived_states[derived_fields::inv_rho][cell_idx] = 1.0f / val;
}

void solver::set_u(i32 cell_idx, f32 val)
{
    conserved_states[conserved_fields::rhou][cell_idx] = val * conserved_states[conserved_fields::rho][cell_idx];
}

void solver::set_v(i32 cell_idx, f32 val)
{
    conserved_states[conserved_fields::rhov][cell_idx] = val * conserved_states[conserved_fields::rho][cell_idx];
}

void solver::set_p(i32 cell_idx, f32 val)
{
    f32 u = conserved_states[conserved_fields::rhou][cell_idx] * derived_states[derived_fields::inv_rho][cell_idx];
    f32 v = conserved_states[conserved_fields::rhov][cell_idx] * derived_states[derived_fields::inv_rho][cell_idx];

    conserved_states[conserved_fields::e][cell_idx] = ideal_gas_law_e(val, conserved_states[conserved_fields::rho][cell_idx], u, v);
}

std::span<const f32> solver::get_rho() const
{
    return conserved_states[conserved_fields::rho];
}

std::span<const f32> solver::get_rhou() const
{
    return conserved_states[conserved_fields::rhou];
}

std::span<const f32> solver::get_rhov() const
{
    return conserved_states[conserved_fields::rhov];
}

std::span<const f32> solver::get_e() const
{
    return conserved_states[conserved_fields::e];
}

std::span<const f32> solver::get_rho(i32 start, i32 count) const
{
    return std::span<const f32>(conserved_states[conserved_fields::rho]).subspan(start, count);
}

std::span<const f32> solver::get_rhou(i32 start, i32 count) const
{
    return std::span<const f32>(conserved_states[conserved_fields::rhou]).subspan(start, count);
}

std::span<const f32> solver::get_rhov(i32 start, i32 count) const
{
    return std::span<const f32>(conserved_states[conserved_fields::rhov]).subspan(start, count);
}

std::span<const f32> solver::get_e(i32 start, i32 count) const
{
    return std::span<const f32>(conserved_states[conserved_fields::e]).subspan(start, count);
}

f32 solver::get_gamma() const
{
    return gamma;
}

f32 solver::get_r() const
{
    return r;
}

f32 solver::get_time_elapsed() const
{
    return time_elapsed;
}

//TODO -> MultiThread
void solver::calculate_derived_states()
{
    using enum conserved_fields;
    using enum derived_fields;

    //has to be taken out of the loop for vectorisation
    const std::size_t num_cells = scene_mesh.get_cells_size_flat();

    for (std::size_t i = 0; i < num_cells; ++i)
    {
        derived_states[inv_rho][i] = 1.0f / conserved_states[rho][i];
        derived_states[v][i] = conserved_states[rhov][i] * derived_states[inv_rho][i];
        derived_states[u][i] = conserved_states[rhou][i] * derived_states[inv_rho][i];
        derived_states[p][i] = ideal_gas_law_p(conserved_states[e][i], conserved_states[rho][i], derived_states[u][i], derived_states[v][i]);
        derived_states[c][i] = std::sqrtf(gamma * derived_states[p][i] * derived_states[inv_rho][i]);
    }
}

void solver::calculate_time_step()
{
    using enum derived_fields;

    f32 min_time_step = std::numeric_limits<f32>::max();

    const f32 dy = scene_mesh.get_dy();
    const f32 dx = scene_mesh.get_dx();

    for (i32 i = 0; i < scene_mesh.get_cells_size_flat(); ++i)
    {
        const f32 cfl_time_step_x = cfl_condition(dx, derived_states[u][i], derived_states[c][i]);
        const f32 cfl_time_step_y = cfl_condition(dy, derived_states[v][i], derived_states[c][i]);

        const f32 cfl_time_step = std::min(cfl_time_step_x, cfl_time_step_y);
        min_time_step = std::min(min_time_step, cfl_time_step);
    }

    dt = min_time_step;
}

void solver::calculate_face_flux_transfer()
{
    for (const boundary_region& region : scene_mesh.get_boundary_regions())
    {
        boundary_region::boundary_types type = region.get_type();
        std::span<const i32> faces = region.get_faces();

        switch (type)
        {
        case(boundary_region::boundary_types::none):
        {
            calculate_none_region(region);
            break;
        }
        case(boundary_region::boundary_types::slip_wall):
        {
            calculate_slip_wall_region(region);
            break;
        }
        case(boundary_region::boundary_types::supersonic_inflow):
        {
            calculate_supersonic_inflow_region(region);
            break;
        }
        case(boundary_region::boundary_types::subsonic_inflow):
        {
            calculate_subsonic_inflow_region(region);
            break;
        }
        case(boundary_region::boundary_types::supersonic_outflow):
        {
            calculate_supersonic_outflow_region(region);
            break;
        }
        case(boundary_region::boundary_types::subsonic_outflow):
        {
            calculate_subsonic_outflow_region(region);
            break;
        }
        }
    }
}

void solver::calculate_none_region(const boundary_region& region)
{
    using enum conserved_fields;
    using enum derived_fields;
    using enum flux_fields;

    const std::span<const i32> faces = region.get_faces();

    for (std::size_t i = 0; i < faces.size(); ++i)
    {
        const i32 face_idx = faces[i];
        const i32 left_state = scene_mesh.get_left_cell(face_idx);
        const i32 right_state = scene_mesh.get_right_cell(face_idx);
        const vec2 normal = scene_mesh.get_normal(face_idx);
        const f32 dt_div_dl = dt * scene_mesh.get_invdl(face_idx);

        const f32 left_normal_velocity = derived_states[u][left_state] * normal.x + derived_states[v][left_state] * normal.y;

        const f32 left_flux[4] = {
            conserved_states[rho][left_state] * left_normal_velocity,
            conserved_states[rhou][left_state] * left_normal_velocity + derived_states[p][left_state] * normal.x,
            conserved_states[rhov][left_state] * left_normal_velocity + derived_states[p][left_state] * normal.y,
            (conserved_states[e][left_state] + derived_states[p][left_state]) * left_normal_velocity
        };

        const f32 right_normal_velocity = derived_states[u][right_state] * normal.x + derived_states[v][right_state] * normal.y;

        const f32 right_flux[4] = {
            conserved_states[rho][right_state] * right_normal_velocity,
            conserved_states[rhou][right_state] * right_normal_velocity + derived_states[p][right_state] * normal.x,
            conserved_states[rhov][right_state] * right_normal_velocity + derived_states[p][right_state] * normal.y,
            (conserved_states[e][right_state] + derived_states[p][right_state]) * right_normal_velocity
        };

        const f32 alpha = std::max(std::abs(left_normal_velocity) + derived_states[c][left_state],
            std::abs(right_normal_velocity) + derived_states[c][right_state]);

        const f32 delta_flux[4] =
        {
            rusanov(left_flux[0], right_flux[0], alpha, conserved_states[rho][left_state], conserved_states[rho][right_state]),
            rusanov(left_flux[1], right_flux[1], alpha, conserved_states[rhou][left_state], conserved_states[rhou][right_state]),
            rusanov(left_flux[2], right_flux[2], alpha, conserved_states[rhov][left_state], conserved_states[rhov][right_state]),
            rusanov(left_flux[3], right_flux[3], alpha, conserved_states[e][left_state], conserved_states[e][right_state]),
        };

        conserved_states_temp[rho][left_state] -= delta_flux[0] * dt_div_dl;
        conserved_states_temp[rhou][left_state] -= delta_flux[1] * dt_div_dl;
        conserved_states_temp[rhov][left_state] -= delta_flux[2] * dt_div_dl;
        conserved_states_temp[e][left_state] -= delta_flux[3] * dt_div_dl;

        conserved_states_temp[rho][right_state] += delta_flux[0] * dt_div_dl;
        conserved_states_temp[rhou][right_state] += delta_flux[1] * dt_div_dl;
        conserved_states_temp[rhov][right_state] += delta_flux[2] * dt_div_dl;
        conserved_states_temp[e][right_state] += delta_flux[3] * dt_div_dl;
    }
}

void solver::calculate_slip_wall_region(const boundary_region& region)
{
    using enum conserved_fields;
    using enum derived_fields;
    using enum flux_fields;

    const i32* __restrict faces = region.get_faces().data();
    const i32* __restrict left_cells = scene_mesh.get_left_cells().data();
    const i32* __restrict right_cells = scene_mesh.get_right_cells().data();
    const vec2* __restrict normals = scene_mesh.get_normals().data();
    const f32* __restrict invdls = scene_mesh.get_invdls().data();

    const std::size_t size = region.get_faces().size();

    for (std::size_t i = 0; i < size; ++i)
    {
        const i32 face_idx = faces[i];

        const i32 left_state = left_cells[face_idx];
        const i32 right_state = right_cells[face_idx];
        const vec2 normal = normals[face_idx];
        const f32 dt_div_dl = dt * invdls[face_idx];

        conserved_states_temp[rhou][left_state] -= derived_states[p][left_state] * normal.x * dt_div_dl;
        conserved_states_temp[rhov][left_state] -= derived_states[p][left_state] * normal.y * dt_div_dl;
        conserved_states_temp[rhou][right_state] += derived_states[p][right_state] * normal.x * dt_div_dl;
        conserved_states_temp[rhov][right_state] += derived_states[p][right_state] * normal.y * dt_div_dl;
    }
}

void solver::calculate_supersonic_inflow_region(const boundary_region& region) {}
void solver::calculate_subsonic_inflow_region(const boundary_region& region) {}
void solver::calculate_supersonic_outflow_region(const boundary_region& region) {}
void solver::calculate_subsonic_outflow_region(const boundary_region& region) {}