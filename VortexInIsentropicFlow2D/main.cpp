#include <iostream>
#include <format>

#include <mesh.h>
#include <boundary_region.h>
#include <solver.h>
#include <riemann_solver.h>
#include <mesh_structs.h>

const std::string file_path = { "C:\\Users\\TomCo\\Desktop\\SimOutput\\" };

constexpr f32 vortex_strength = 5.0f;
constexpr f32 horizontal_flow_speed = 1.0f;
constexpr f32 vertical_flow_speed = 1.0f;
constexpr f32 base_pressure = 1.0f;
constexpr f32 gamma = 1.4f;
constexpr f32 pi = 3.14159265359f;

constexpr f32 vortex_center_x = 5.0f;
constexpr f32 vortex_center_y = 5.0f;

f32 calculate_radius_squared(f32 x, f32 y)
{
    f32 dx = x - vortex_center_x;
    f32 dy = y - vortex_center_y;

    return dx * dx + dy * dy;
}

f32 calculate_temperature(f32 x, f32 y)
{
    f32 radius_squared = calculate_radius_squared(x, y);

    return 1.0f -
        ((gamma - 1.0f) * vortex_strength * vortex_strength) /
        (8.0f * gamma * pi * pi) *
        std::exp(1.0f - radius_squared);
}

f32 calculate_rho(f32 x, f32 y)
{
    f32 temperature = calculate_temperature(x, y);
    f32 exponent = 1.0f / (gamma - 1.0f);

    return std::pow(temperature, exponent);
}

f32 calculate_u(f32 x, f32 y)
{
    f32 dx = x - vortex_center_x;
    f32 dy = y - vortex_center_y;
    f32 radius_squared = dx * dx + dy * dy;

    f32 perturbation =
        -(vortex_strength / (2.0f * pi)) *
        dy *
        std::exp((1.0f - radius_squared) / 2.0f);

    return horizontal_flow_speed + perturbation;
}

f32 calculate_v(f32 x, f32 y)
{
    f32 dx = x - vortex_center_x;
    f32 dy = y - vortex_center_y;
    f32 radius_squared = dx * dx + dy * dy;

    f32 perturbation =
        (vortex_strength / (2.0f * pi)) *
        dx *
        std::exp((1.0f - radius_squared) / 2.0f);

    return vertical_flow_speed + perturbation;
}

f32 calculate_p(f32 x, f32 y)
{
    f32 temperature = calculate_temperature(x, y);
    f32 exponent = gamma / (gamma - 1.0f);

    return base_pressure * std::pow(temperature, exponent);
}

bool is_face_in_fluid_region(const mesh_structs::face_info& face_info, const mesh_structs::mesh_info& mesh_info)
{
    return true;;
}

vec4 run_vortex_in_isentropic_flow(std::string sim_name, std::size_t resolution)
{    
    mesh_structs::boundary_config fluid_region = {
        .type = region_structs::types::none,
        .face_in_region = is_face_in_fluid_region,
    };
    
    mesh_structs::config scene_mesh_config = {
        .resolution = {resolution, resolution},
        .dimensions = {10.0f, 10.0f},
        .vertically_periodic = true,
        .horizontally_periodic = true,
        .region_configs = {fluid_region}
    };

    solver::config solver_config = {
        .mesh_config = scene_mesh_config,
        .r = 1.0f,
        .gamma = 1.4f,
        .cfl_target = 0.5f,
        .scene_time = 10.0f,
    };

    solver scene_solver{ solver_config };

    const mesh& scene_mesh = scene_solver.get_mesh();

    std::vector<f32> initual_rho(scene_mesh.cells_size_flat());
    std::vector<f32> initual_u(scene_mesh.cells_size_flat());
    std::vector<f32> initual_v(scene_mesh.cells_size_flat());
    std::vector<f32> initual_p(scene_mesh.cells_size_flat());

    for (std::size_t i = 0; i < scene_mesh.cells_size_flat(); ++i)
    {
        u64vec2 cell_coords = scene_mesh.get_cell_position(i);
        f32 x = ((f32)cell_coords.x + 0.5f) * scene_mesh.get_dx();
        f32 y = ((f32)cell_coords.y + 0.5f)* scene_mesh.get_dy();

        solver::primitive_state state = {
            .rho = calculate_rho(x, y),
            .u = calculate_u(x, y),
            .v = calculate_v(x, y),
            .p = calculate_p(x, y)
        };

        initual_rho[i] = state.rho;
        initual_u[i] = state.u;
        initual_v[i] = state.v;
        initual_p[i] = state.p;

        scene_solver.set_primitive_state(i, state);
    }

   
    std::size_t num_ticks = 0;
    f32 time_elapsed = 0.0f;
    while (time_elapsed < scene_solver.total_time())
    {
        if (num_ticks % 20 == 0)
        {
            time_elapsed = scene_solver.time_elapsed();
            f32 percentage_complete = 100.f * time_elapsed / scene_solver.total_time();
            std::cout << sim_name << " Is Percentage Complete: " << percentage_complete << "%\n";
        }
   
        scene_solver.time_step();
        ++num_ticks;
    }

    std::vector<f32> scene_rho = scene_solver.get_rho();
    std::vector<f32> scene_rhou = scene_solver.get_rhou();
    std::vector<f32> scene_rhov = scene_solver.get_rhov();
    std::vector<f32> scene_e = scene_solver.get_e();

    std::vector<f32> scene_u(scene_rho.size());
    std::vector<f32> scene_v(scene_rho.size());
    std::vector<f32> scene_p(scene_rho.size());

    f32 dx = scene_mesh.get_dx();
    f32 dy = scene_mesh.get_dy();
    
    vec4 residuals = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (i32 i = 0; i < scene_mesh.cells_size_flat(); ++i)
    {
        scene_u[i] = scene_rhou[i] / scene_rho[i];
        scene_v[i] = scene_rhov[i] / scene_rho[i];
        scene_p[i] = scene_solver.ideal_gas_law_p(scene_e[i], scene_rho[i], scene_u[i], scene_v[i]);
    }

    for (std::size_t i = 0; i < scene_mesh.cells_size_flat(); ++i)
    {
        u64vec2 cell_coords = scene_mesh.get_cell_position(i);

        residuals.x += dx * dy * std::abs(scene_rho[i] - initual_rho[i]);
        residuals.y += dx * dy * std::abs(scene_u[i] - initual_u[i]);
        residuals.z += dx * dy * std::abs(scene_v[i] - initual_v[i]);
        residuals.w += dx * dy * std::abs(scene_p[i] - initual_p[i]);
    }

    return residuals;
}

void print_residual(std::string sim_name, const vec4& residual)
{
    std::cout << "\n" << sim_name << "\n";
    std::cout << "Rho Residual: " << residual.x << "\n";
    std::cout << "u Residual: " << residual.y << "\n";
    std::cout << "v Residual: " << residual.z << "\n";
    std::cout << "Pressure Residual: " << residual.w << "\n";
}

int main()
{
    vec4 sim_125 = run_vortex_in_isentropic_flow("vortex_in_isentropic_flow_125", 125);
    vec4 sim_250 = run_vortex_in_isentropic_flow("vortex_in_isentropic_flow_250", 250);
    vec4 sim_500 = run_vortex_in_isentropic_flow("vortex_in_isentropic_flow_500", 500);
    vec4 sim_1000 = run_vortex_in_isentropic_flow("vortex_in_isentropic_flow_1000", 1000);
    //vec4 sim_2000 = run_vortex_in_isentropic_flow("vortex_in_isentropic_flow_2000", 2000);

    print_residual("vortex_in_isentropic_flow_125", sim_125);
    print_residual("vortex_in_isentropic_flow_250", sim_250);
    print_residual("vortex_in_isentropic_flow_500", sim_500);
    print_residual("vortex_in_isentropic_flow_1000", sim_1000);
    //print_residual("vortex_in_isentropic_flow_2000", sim_2000);

    return 0;
}
