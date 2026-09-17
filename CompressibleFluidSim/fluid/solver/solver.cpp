#include "CompressibleFluidSim/fluid/solver/solver.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <span>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <iostream>

solver::solver(const config& config) : mesh(config.mesh_config)
{
    r = config.r;
    gamma = config.gamma;
    cfl_target = config.cfl_target;

    // Init ConservedState
    i32 num_cells = mesh.cells_size_flat();
    conserved_states.resize(num_cells, 0.0f);
    
    //hacky, should be better;
    f32 null = 0.0f;
    _device_dt.upload(&null, 1);
    _device_time_elapsed.upload(&null, 1);
    _device_lambda.upload(&null, 1);
    _device_total_time.upload(&config.scene_time, 1);
    _host_total_time = config.scene_time;
}

void solver::time_step()
{
    if (!moved_to_device)
    {
        move_to_device();
    }
    
    calculate_derived_states();
    
    calculate_time_step();

    std::copy(device_conserved_states.begin(), device_conserved_states.end(), device_conserved_states_temp.begin());

    calculate_face_flux_transfer();

    std::swap(device_conserved_states_temp, device_conserved_states);
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

void solver::set_primitive_state(std::size_t cell_idx, const primitive_state& state)
{
    const f32 inv_rho = 1.0f / state.rho;
    const f32 e = ideal_gas_law_e(state.p, state.rho, state.u, state.v);
    const f32 c = std::sqrt(gamma * state.p * inv_rho);

    conserved_states[conserved_fields::rho][cell_idx] = state.rho;
    conserved_states[conserved_fields::rhou][cell_idx] = state.rho * state.u;
    conserved_states[conserved_fields::rhov][cell_idx] = state.rho * state.v;
    conserved_states[conserved_fields::e][cell_idx] = e;
}

void solver::set_conserved_state(std::size_t cell_idx, const conserved_state& state)
{
    conserved_states[conserved_fields::rho][cell_idx] = state.rho;
    conserved_states[conserved_fields::rhou][cell_idx] = state.rhou;
    conserved_states[conserved_fields::rhov][cell_idx] = state.rhov;
    conserved_states[conserved_fields::e][cell_idx] = state.e;
}

std::vector<f32> solver::get_rho() const
{
    std::vector<f32> temp(mesh.cells_size_flat());

    device_conserved_states[conserved_fields::rho].download(temp.data());

    return temp;
}

std::vector<f32> solver::get_rhou() const
{
    std::vector<f32> temp(mesh.cells_size_flat());

    device_conserved_states[conserved_fields::rhou].download(temp.data());

    return temp;
}

std::vector<f32> solver::get_rhov() const
{
    std::vector<f32> temp(mesh.cells_size_flat());

    device_conserved_states[conserved_fields::rhov].download(temp.data());

    return temp;
}

std::vector<f32> solver::get_e() const
{
    std::vector<f32> temp(mesh.cells_size_flat());

    device_conserved_states[conserved_fields::e].download(temp.data());

    return temp;
}

f32 solver::get_gamma() const
{
    return gamma;
}

f32 solver::get_r() const
{
    return r;
}

f32 solver::time_elapsed() const
{
    f32 buffer;
    _device_time_elapsed.download(&buffer);

    return buffer;
}

f32 solver::total_time() const
{
    return _host_total_time;
}

const mesh& solver::get_mesh() const
{
    return mesh;
}

void solver::move_to_device()
{
    using enum conserved_fields;

    std::size_t num_cells = mesh.cells_size_flat();
    std::size_t num_faces = mesh.faces_size();

    device_conserved_states_temp.resize(num_cells);
    device_derived_states.resize(num_cells);
    device_fluxes.resize(num_faces);
    
    device_conserved_states[rho] = gpu_buffer<f32>{conserved_states[rho].data(),  num_cells};
    device_conserved_states[rhou] = gpu_buffer<f32>{ conserved_states[rhou].data(),  num_cells};
    device_conserved_states[rhov] = gpu_buffer<f32>{ conserved_states[rhov].data(),  num_cells};
    device_conserved_states[e] = gpu_buffer<f32>{ conserved_states[e].data(), num_cells};
    
    moved_to_device = true;
}

void solver::calculate_face_flux_transfer()
{
    for (const boundary_region& region : mesh.boundary_regions())
    {
        region_structs::types type = region.type();

        switch (type)
        {
        case(region_structs::types::none):
        {
            calculate_none_region(region);
            break;
        }
        case(region_structs::types::slip_wall):
        {
            calculate_slip_wall_region(region);
            break;
        }
        case(region_structs::types::supersonic_inflow):
        {
            //calculate_supersonic_inflow_region(region);
            break;
        }
        case(region_structs::types::subsonic_inflow):
        {
            //calculate_subsonic_inflow_region(region);
            break;
        }
        case(region_structs::types::supersonic_outflow):
        {
            //calculate_supersonic_outflow_region(region);
            break;
        }
        case(region_structs::types::subsonic_outflow):
        {
            //calculate_subsonic_outflow_region(region);
            break;
        }
        }
    }
}

void solver::calculate_supersonic_inflow_region(const boundary_region& region) {}
void solver::calculate_subsonic_inflow_region(const boundary_region& region) {}
void solver::calculate_supersonic_outflow_region(const boundary_region& region) {}
void solver::calculate_subsonic_outflow_region(const boundary_region& region) {}