#pragma once
#include <vector>
#include <memory>
#include <string>

#include "CompressibleFluidSim/fluid/utils/types.h"
#include "CompressibleFluidSim/fluid/utils/gpu_struct_of_arrays.h"
#include "CompressibleFluidSim/fluid/utils/struct_of_vectors.h"
#include "CompressibleFluidSim/fluid/mesh/mesh.h"

class solver
{
public:
    struct config
    {
        mesh_structs::config mesh_config;
        f32 r = 1.0f;
        f32 gamma = 1.4f;
        f32 cfl_target = 0.5f;
        f32 scene_time = 0.0f;
    };

    enum class conserved_fields : std::size_t
    {
        rho,
        rhou,
        rhov,
        e
    };

    enum class derived_fields : std::size_t
    {
        inv_rho,
        u,
        v,
        p,
        c,
    };

    enum class flux_fields : std::size_t
    {
        mass,
        momentum_u,
        momentum_v,
        e,
    };

    struct primitive_state
    {
        f32 rho;
        f32 u;
        f32 v;
        f32 p;
    };

    struct conserved_state
    {
        f32 rho;
        f32 rhou;
        f32 rhov;
        f32 e;
    };

    solver(const config& config);

    void time_step();
    
    //Math Helpers
    f32 ideal_gas_law_e(f32 p, f32 rho, f32 u, f32 v) const;
    f32 ideal_gas_law_p(f32 e, f32 rho, f32 u, f32 v) const;
    f32 cfl_condition(f32 dx, f32 u, f32 a) const;
    f32 rusanov(f32 flux_left, f32 flux_right, f32 alpha, f32 conserved_left, f32 conserved_right) const;

    void set_primitive_state(std::size_t cell_idx, const primitive_state& state);
    void set_conserved_state(std::size_t cell_idx, const conserved_state& state);

    std::vector<f32> get_rho() const;
    std::vector<f32> get_rhou() const;
    std::vector<f32> get_rhov() const;
    std::vector<f32> get_e() const;

    f32 get_gamma() const;
    f32 get_r() const;

    //gets the total time the scene has been running for, causes a copy from the gpu, so is slow.
    f32 time_elapsed() const;
    f32 total_time() const;

    const mesh& get_mesh() const;

private:
    sov<f32, conserved_fields, 4, 64> conserved_states;
    //sov<f32, conserved_fields, 4, 64> conserved_states_temp;
    //sov<f32, derived_fields, 5, 64> derived_states;
    //sov<f32, flux_fields, 4, 64> fluxes; -> move to gpu buffer only

    gpu_struct_of_arrays<f32, conserved_fields, 4> device_conserved_states;
    gpu_struct_of_arrays<f32, conserved_fields, 4> device_conserved_states_temp;
    gpu_struct_of_arrays<f32, derived_fields, 5> device_derived_states;
    gpu_struct_of_arrays<f32, flux_fields, 4> device_fluxes;
    
    gpu_buffer<f32> _device_dt;
    gpu_buffer<f32> _device_time_elapsed;
    gpu_buffer<f32> _device_lambda;
    gpu_buffer<f32> _device_total_time;

    f32 _host_total_time = 0.0f;

    bool moved_to_device = false;

    const mesh mesh;

    f32 r = 0.0f;
    f32 gamma = 0.0f;
    f32 cfl_target = 0.0f;


    void move_to_device();

    void calculate_derived_states();
    void calculate_time_step();
    void calculate_face_flux_transfer();

    void calculate_none_region(const boundary_region& region);
    void calculate_slip_wall_region(const boundary_region& region);
    void calculate_supersonic_inflow_region(const boundary_region& region);
    void calculate_subsonic_inflow_region(const boundary_region& region);
    void calculate_supersonic_outflow_region(const boundary_region& region);
    void calculate_subsonic_outflow_region(const boundary_region& region);
};