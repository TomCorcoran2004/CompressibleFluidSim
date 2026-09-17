#include <cstddef>
#include <cuda_runtime.h>
#include "CompressibleFluidSim/fluid/utils/gpu_struct_of_arrays.h"
#include "CompressibleFluidSim/fluid/solver/solver.h"
#include "CompressibleFluidSim/fluid/utils/cuda_utils.cuh"

constexpr int blocks_size = 256;

__global__ void calculate_derived_states_kernel(
    f32* rho, 
    f32* rhou, 
    f32* rhov, 
    f32* e,
    f32* inv_rho,
    f32* u,
    f32* v,
    f32* p,
    f32* c,
    f32 gamma,
    std::size_t n
)
{
    std::size_t i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i >= n)
        return;

    inv_rho[i] = 1.0f / rho[i];
    u[i] = rhou[i] * inv_rho[i];
    v[i] = rhov[i] * inv_rho[i];
    p[i] = (gamma - 1.0f) * (e[i] - 0.5f * rho[i] * (u[i] * u[i] + v[i] * v[i]));
    c[i] = sqrtf(gamma * p[i] * inv_rho[i]);
}

void solver::calculate_derived_states()
{
    using enum conserved_fields;
    using enum derived_fields;
    
    const std::size_t size = mesh.cells_size_flat();
    const int num_blocks = (size + blocks_size - 1) / blocks_size;

    calculate_derived_states_kernel<<<num_blocks, blocks_size>>>(
        device_conserved_states[rho].data(),
        device_conserved_states[rhou].data(),
        device_conserved_states[rhov].data(),
        device_conserved_states[e].data(),
        device_derived_states[inv_rho].data(),
        device_derived_states[u].data(),
        device_derived_states[v].data(),
        device_derived_states[p].data(),
        device_derived_states[c].data(),
        gamma,
        size
    );
}

__global__ void calculate_lambda_kernel(f32 inv_dx, f32 inv_dy, f32* u, f32* v, f32* c, std::size_t n, f32* global_lambda)
{
    //assumption that block size is always power of 2
    //must always set global lambda to 0 before launching

    __shared__ f32 lambdas[blocks_size];
    
    std::size_t i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i >= n)
        lambdas[threadIdx.x] = 0.0f;
    else
        lambdas[threadIdx.x] = (fabsf(u[i]) + c[i]) * inv_dx + (fabsf(v[i]) + c[i]) * inv_dy;

    __syncthreads();

    for (std::size_t stride = blocks_size / 2; stride > 0; stride /= 2)
    {
        if (threadIdx.x < stride)
        {
            lambdas[threadIdx.x] = fmaxf(lambdas[threadIdx.x], lambdas[threadIdx.x + stride]);
        }

        __syncthreads();
    }

    if (threadIdx.x == 0)
    {
        atomic_max_f32(global_lambda, lambdas[0]);
    }
}

__global__ void calculate_time_step_kernel(f32* global_lambda, f32* global_dt, f32* global_time_elapsed, f32* total_scene_time, f32 cfl_target)
{
    if (threadIdx.x == 0 && blockIdx.x == 0)
    {
        if (*global_time_elapsed >= *total_scene_time)
        {
            *global_dt = 0.0f;
            return;
        }
        else
        {
            *global_dt = cfl_target / *global_lambda;
            *global_lambda = 0.0f;
            *global_time_elapsed += *global_dt;
        }
    }
}

void solver::calculate_time_step()
{
    using enum conserved_fields;
    using enum derived_fields;

    const std::size_t size = mesh.cells_size_flat();
    const int num_blocks = (size + blocks_size - 1) / blocks_size;

    //kernel;
    calculate_lambda_kernel<<<num_blocks, blocks_size>>>(
        1.0f / mesh.get_dx(),
        1.0f / mesh.get_dy(),
        device_derived_states[u].data(),
        device_derived_states[v].data(),
        device_derived_states[c].data(),
        size,
        _device_lambda.data()
    );

    calculate_time_step_kernel<<<1, 1 >>>(
        _device_lambda.data(),
        _device_dt.data(),
        _device_time_elapsed.data(),
        _device_total_time.data(),
        cfl_target
    );
}

__device__ f32 rusanov(f32 flux_left, f32 flux_right, f32 alpha, f32 conserved_left, f32 conserved_right)
{
    return (flux_left + flux_right) * 0.5f - 0.5f * alpha * (conserved_right - conserved_left);
}

__global__ void calculate_none_region_face_flux_kernel(
    const std::size_t* face_idxs,
    const std::size_t num_faces,
    const std::size_t* left_cell_idxs,
    const std::size_t* right_cell_idxs,
    const vec2* normals,
    const f32* inv_dls,
    const f32* rho,
    const f32* rhou,
    const f32* rhov,
    const f32* e,
    const f32* u,
    const f32* v,
    const f32* p,
    const f32* c,
    f32* mass_flux,
    f32* momentumu_flux,
    f32* momentumv_flux,
    f32* e_flux,
    const f32* dt
)
{
    std::size_t i = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (i >= num_faces)
        return;

    const std::size_t face_idx = face_idxs[i];
    const std::size_t left_state = left_cell_idxs[face_idx];
    const std::size_t right_state = right_cell_idxs[face_idx];
    const vec2 normal = normals[face_idx];
    const f32 dt_div_dl = *dt * inv_dls[face_idx];

    const f32 left_normal_velocity = u[left_state] * normal.x + v[left_state] * normal.y;

    const f32 left_flux[4] = {
        rho[left_state] * left_normal_velocity,
        rhou[left_state] * left_normal_velocity + p[left_state] * normal.x,
        rhov[left_state] * left_normal_velocity + p[left_state] * normal.y,
        (e[left_state] + p[left_state]) * left_normal_velocity
    };

    const f32 right_normal_velocity = u[right_state] * normal.x + v[right_state] * normal.y;

    const f32 right_flux[4] = {
        rho[right_state] * right_normal_velocity,
        rhou[right_state] * right_normal_velocity + p[right_state] * normal.x,
        rhov[right_state] * right_normal_velocity + p[right_state] * normal.y,
        (e[right_state] + p[right_state]) * right_normal_velocity
    };

    const f32 alpha = fmaxf(fabsf(left_normal_velocity) + c[left_state],
        fabsf(right_normal_velocity) + c[right_state]);

    const f32 delta_flux[4] =
    {
        rusanov(left_flux[0], right_flux[0], alpha, rho[left_state], rho[right_state]),
        rusanov(left_flux[1], right_flux[1], alpha, rhou[left_state], rhou[right_state]),
        rusanov(left_flux[2], right_flux[2], alpha, rhov[left_state], rhov[right_state]),
        rusanov(left_flux[3], right_flux[3], alpha, e[left_state], e[right_state]),
    };

    mass_flux[face_idx] = delta_flux[0] * dt_div_dl;
    momentumu_flux[face_idx] = delta_flux[1] * dt_div_dl;
    momentumv_flux[face_idx] = delta_flux[2] * dt_div_dl;
    e_flux[face_idx] = delta_flux[3] * dt_div_dl;
}

__global__ void calculate_face_flux_transfer_kernel(
    const std::size_t* __restrict__ cell_idxs,
    const std::size_t* __restrict__ top_faces,
    const std::size_t* __restrict__ bottom_faces,
    const std::size_t* __restrict__ left_faces,
    const std::size_t* __restrict__ right_faces,
    const std::size_t* __restrict__ region_ids,
    const std::size_t num_faces_mesh,
    const std::size_t num_cells_region,
    const std::size_t region_idx,
    f32* __restrict__ rho,
    f32* __restrict__ rhou,
    f32* __restrict__ rhov,
    f32* __restrict__ e,
    const f32* __restrict__ mass_flux,
    const f32* __restrict__ momentumu_flux,
    const f32* __restrict__ momentumv_flux,
    const f32* __restrict__ e_flux
)
{
    std::size_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_cells_region)
        return;

    std::size_t cell_idx = cell_idxs[i];

    f32 delta_rho = 0.0f;
    f32 delta_rhou = 0.0f;
    f32 delta_rhov = 0.0f;
    f32 delta_e = 0.0f;

    std::size_t top_face = top_faces[cell_idx];
    if (top_face < num_faces_mesh && region_idx == region_ids[top_face])
    {
        delta_rho -= mass_flux[top_face];
        delta_rhou -= momentumu_flux[top_face];
        delta_rhov -= momentumv_flux[top_face];
        delta_e -= e_flux[top_face];
    }

    std::size_t bottom_face = bottom_faces[cell_idx];
    if (bottom_face < num_faces_mesh && region_idx == region_ids[bottom_face])
    {
        delta_rho += mass_flux[bottom_face];
        delta_rhou += momentumu_flux[bottom_face];
        delta_rhov += momentumv_flux[bottom_face];
        delta_e += e_flux[bottom_face];
    }

    std::size_t left_face = left_faces[cell_idx];
    if (left_face < num_faces_mesh && region_idx == region_ids[left_face])
    {
        delta_rho += mass_flux[left_face];
        delta_rhou += momentumu_flux[left_face];
        delta_rhov += momentumv_flux[left_face];
        delta_e += e_flux[left_face];
    }

    std::size_t right_face = right_faces[cell_idx];
    if (right_face < num_faces_mesh && region_idx == region_ids[right_face])
    {
        delta_rho -= mass_flux[right_face];
        delta_rhou -= momentumu_flux[right_face];
        delta_rhov -= momentumv_flux[right_face];
        delta_e -= e_flux[right_face];
    }

    rho[cell_idx] += delta_rho;
    rhou[cell_idx] += delta_rhou;
    rhov[cell_idx] += delta_rhov;
    e[cell_idx] += delta_e;
}

void solver::calculate_none_region(const boundary_region& region)
{
    using enum conserved_fields;
    using enum derived_fields;

    const std::size_t num_faces = region.num_faces();
    const std::size_t num_cells = region.num_cells();
    
    const int num_blocks_flux_calculation = (num_faces + blocks_size - 1) / blocks_size;

    calculate_none_region_face_flux_kernel<<<num_blocks_flux_calculation, blocks_size>>>(
        region.device_face_idxs(),
        num_faces,
        mesh.device_left_cells(),
        mesh.device_right_cells(),
        mesh.device_normals(),
        mesh.device_inv_dls(),
        device_conserved_states[rho].data(),
        device_conserved_states[rhou].data(),
        device_conserved_states[rhov].data(),
        device_conserved_states[e].data(),
        device_derived_states[u].data(),
        device_derived_states[v].data(),
        device_derived_states[p].data(),
        device_derived_states[c].data(),
        device_fluxes[flux_fields::mass].data(),
        device_fluxes[flux_fields::momentum_u].data(),
        device_fluxes[flux_fields::momentum_v].data(),
        device_fluxes[flux_fields::e].data(),
        _device_dt.data()
    );

    const int num_blocks_flux_transfer = (num_cells + blocks_size - 1) / blocks_size;

    calculate_face_flux_transfer_kernel << <num_blocks_flux_transfer, blocks_size >> > (
        region.device_cell_idxs(),
        mesh.device_top_face_idxs(),
        mesh.device_bottom_face_idxs(),
        mesh.device_left_face_idxs(),
        mesh.device_right_face_idxs(),
        mesh.device_region_ids(),
        mesh.faces_size(),
        num_cells,
        region.id(),
        device_conserved_states_temp[rho].data(),
        device_conserved_states_temp[rhou].data(),
        device_conserved_states_temp[rhov].data(),
        device_conserved_states_temp[e].data(),
        device_fluxes[flux_fields::mass].data(),
        device_fluxes[flux_fields::momentum_u].data(),
        device_fluxes[flux_fields::momentum_v].data(),
        device_fluxes[flux_fields::e].data()
    );
}

__global__ void calculate_slip_wall_region_kernel(
    const std::size_t* cell_idxs,
    const std::size_t* top_faces,
    const std::size_t* bottom_faces,
    const std::size_t* left_faces,
    const std::size_t* right_faces,
    const std::size_t* region_ids,
    const std::size_t num_faces_mesh,
    const std::size_t num_cells_region,
    const std::size_t region_idx,
    f32* rhou,
    f32* rhov,
    const f32* p,
    const vec2* normals,
    const f32* inv_dls,
    const f32* dt
)
{
    std::size_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_cells_region)
        return;

    std::size_t cell_idx = cell_idxs[i];
    f32 pressure = p[cell_idx];

    std::size_t top_face = top_faces[cell_idx];
    if (top_face < num_faces_mesh && region_idx == region_ids[top_face])
    {
        vec2 normal = normals[top_face];
        const f32 dt_div_dl = *dt * inv_dls[top_face];
        rhou[cell_idx] -= normal.x * pressure * dt_div_dl;
        rhov[cell_idx] -= normal.y * pressure * dt_div_dl;
    }

    std::size_t bottom_face = bottom_faces[cell_idx];
    if (bottom_face < num_faces_mesh && region_idx == region_ids[bottom_face])
    {
        vec2 normal = normals[bottom_face];
        const f32 dt_div_dl = *dt * inv_dls[bottom_face];
        rhou[cell_idx] += normal.x * pressure * dt_div_dl;
        rhov[cell_idx] += normal.y * pressure * dt_div_dl;
    }

    std::size_t left_face = left_faces[cell_idx];
    if (left_face < num_faces_mesh && region_idx == region_ids[left_face])
    {
        vec2 normal = normals[left_face];
        const f32 dt_div_dl = *dt * inv_dls[left_face];
        rhou[cell_idx] += normal.x * pressure * dt_div_dl;
        rhov[cell_idx] += normal.y * pressure * dt_div_dl;
    }

    std::size_t right_face = right_faces[cell_idx];
    if (right_face < num_faces_mesh && region_idx == region_ids[right_face])
    {
        vec2 normal = normals[right_face];
        const f32 dt_div_dl = *dt * inv_dls[right_face];
        rhou[cell_idx] -= normal.x * pressure * dt_div_dl;
        rhov[cell_idx] -= normal.y * pressure * dt_div_dl;
    }
}

void solver::calculate_slip_wall_region(const boundary_region& region)
{
    using enum conserved_fields;
    using enum derived_fields;

    const std::size_t num_cells = region.num_cells();
    const int num_blocks_flux_transfer = (num_cells + blocks_size - 1) / blocks_size;

    calculate_slip_wall_region_kernel<<<num_blocks_flux_transfer, blocks_size>>>(
        region.device_cell_idxs(),
        mesh.device_top_face_idxs(),
        mesh.device_bottom_face_idxs(),
        mesh.device_left_face_idxs(),
        mesh.device_right_face_idxs(),
        mesh.device_region_ids(),
        mesh.faces_size(),
        num_cells,
        region.id(),
        device_conserved_states_temp[rhou].data(),
        device_conserved_states_temp[rhov].data(),
        device_derived_states[p].data(),
        mesh.device_normals(),
        mesh.device_inv_dls(),
        _device_dt.data()
    );
}