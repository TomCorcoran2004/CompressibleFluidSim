#pragma once
#include <vector>
#include <memory>
#include <string>

#include "types.h"
#include "struct_of_vectors.h"
#include "mesh.h"

class solver
{
public:
    struct config
    {
        mesh::config mesh_config;
        f32 r = 1.0f;
        f32 gamma = 1.4f;
        f32 cfl_target = 0.5f;
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
    void write_vti_ascii(const std::string& filepath, const std::string& filename);
    void write_vti_binary(const std::string& filepath, const std::string& filename);

    //Math Helpers
    f32 ideal_gas_law_e(f32 p, f32 rho, f32 u, f32 v) const;
    f32 ideal_gas_law_p(f32 e, f32 rho, f32 u, f32 v) const;
    f32 cfl_condition(f32 dx, f32 u, f32 a) const;
    f32 rusanov(f32 flux_left, f32 flux_right, f32 alpha, f32 conserved_left, f32 conserved_right) const;

    void set_primitive_state(std::size_t cell_idx, const primitive_state& state);
    void set_conserved_state(std::size_t cell_idx, const conserved_state& state);

    primitive_state get_primitive_state(std::size_t cell_idx);
    conserved_state get_conserved_state(std::size_t cell_idx);

    void set_rho(std::size_t cell_idx, f32 val);
    void set_u(std::size_t cell_idx, f32 val);
    void set_v(std::size_t cell_idx, f32 val);
    void set_p(std::size_t cell_idx, f32 val);

    void add_rho(std::size_t cell_idx, f32 val);
    void add_u(std::size_t cell_idx, f32 val);
    void add_v(std::size_t cell_idx, f32 val);
    void add_p(std::size_t cell_idx, f32 val);

    std::span<const f32> get_rho() const;
    std::span<const f32> get_rhou() const;
    std::span<const f32> get_rhov() const;
    std::span<const f32> get_e() const;

    std::span<const f32> get_rho(i32 start, i32 count) const;
    std::span<const f32> get_rhou(i32 start, i32 count) const;
    std::span<const f32> get_rhov(i32 start, i32 count) const;
    std::span<const f32> get_e(i32 start, i32 count) const;

    f32 get_gamma() const;
    f32 get_r() const;
    f32 get_time_elapsed() const;

    const mesh& get_mesh() const;

private:
    sov<f32, conserved_fields, 4, 64> conserved_states;
    sov<f32, conserved_fields, 4, 64> conserved_states_temp;
    sov<f32, derived_fields, 5, 64> derived_states;
    sov<f32, flux_fields, 4, 64> fluxes;

    const mesh scene_mesh;

    f32 dt = 0.0f;
    f32 r = 0.0f;
    f32 gamma = 0.0f;
    f32 cfl_target = 0.0f;

    f32 time_elapsed = 0.0f;

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