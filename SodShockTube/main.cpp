#include <iostream>

#include <mesh.h>
#include <boundary_region.h>
#include <solver.h>
#include <riemann_solver.h>

bool IsFaceInEdgeRegion(const boundary_region::face_info& face_info, const boundary_region::mesh_info& mesh_info)
{
    i32 mesh_size_x = mesh_info.mesh_resolution.x;
    i32 mesh_size_y = mesh_info.mesh_resolution.y;

    ivec2 left_cell_pos = { face_info.left_cell % mesh_size_x , face_info.left_cell / mesh_size_x };
    ivec2 right_cell_pos = { face_info.right_cell % mesh_size_x , face_info.right_cell / mesh_size_x };

    if (left_cell_pos.x == 0 || right_cell_pos.x == 0)
        return true;

    if (left_cell_pos.x == mesh_size_x - 1 || right_cell_pos.x == mesh_size_x - 1)
        return true;

    if (left_cell_pos.y == 0 || right_cell_pos.y == 0)
        return true;

    if (left_cell_pos.y == mesh_size_y - 1 || right_cell_pos.y == mesh_size_y - 1)
        return true;

    return false;
}

bool IsFaceInFluidRegion(const boundary_region::face_info& face_info, const boundary_region::mesh_info& mesh_info)
{
    return !IsFaceInEdgeRegion(face_info, mesh_info);
}

riemann_solver::w run_sod_shock(std::string sim_name, i32 resolution)
{
    mesh::config scene_mesh_config = {
        .resolution = {resolution, 3},
        .dimensions = {1.0f, 1.0f}
    };
    mesh scene_mesh{ scene_mesh_config };

    boundary_region::boundary_config slip_wall = {
        .name = "SlipWall",
        .type = boundary_region::boundary_types::slip_wall,
        .face_in_region = IsFaceInEdgeRegion,
    };

    boundary_region::boundary_config fluid_region = {
        .name = "FluidRegion",
        .type = boundary_region::boundary_types::none,
        .face_in_region = IsFaceInFluidRegion,
    };

    scene_mesh.add_boundary_region(slip_wall);
    scene_mesh.add_boundary_region(fluid_region);

    solver::config solver_config = {
        .scene_mesh = scene_mesh,
        .r = 1.0f,
        .gamma = 1.4f,
        .cfl_target = 0.5f,
    };

    solver scene_solver{ solver_config };

    for (std::size_t i = 0; i < scene_mesh.get_cells_size_flat(); ++i)
    {
        if ((f32)scene_mesh.get_cell_position(i).x / (f32)scene_mesh.get_cells_size().x < 0.5f)
        {
            scene_solver.set_rho(i, 1.0f);
            scene_solver.set_u(i, 0.0f);
            scene_solver.set_p(i, 1.0f);
        }
        else
        {
            scene_solver.set_rho(i, 0.125f);
            scene_solver.set_u(i, 0.0f);
            scene_solver.set_p(i, 0.1f);
        }
    }

    while (scene_solver.get_time_elapsed() < 0.2f)
    {
        f32 percentage_complete = 100.f * scene_solver.get_time_elapsed() / 0.2f;
        std::cout << sim_name << " Is Percentage Complete: " << percentage_complete << "%\n";
        scene_solver.time_step();
    }

    riemann_solver::w left = {
        .rho = 1.0f,
        .u = 0.0f,
        .p = 1.0f,
    };

    riemann_solver::w right = {
        .rho = 0.125f,
        .u = 0.0f,
        .p = 0.1f,
    };

    riemann_solver::config riemann_config = {
        .left = left,
        .right = right,
        .gamma = 1.4f,
    };

    riemann_solver analytical_solution{ riemann_config };
    
    riemann_solver::w residuals = {
        .rho = 0.0f,
        .u = 0.0f,
        .p = 0.0f,
    };

    std::span<const f32> scene_rho = scene_solver.get_rho();
    std::span<const f32> scene_rhou = scene_solver.get_rhou();
    std::span<const f32> scene_rhov = scene_solver.get_rhov();
    std::span<const f32> scene_e = scene_solver.get_e();

    std::vector<f32> scene_u(scene_rho.size());
    std::vector<f32> scene_v(scene_rho.size());
    std::vector<f32> scene_p(scene_rho.size());

    f32 dx = scene_mesh.get_dx();

    for (i32 i = 0; i < scene_mesh.get_cells_size_flat(); ++i)
    {
        scene_u[i] = scene_rhou[i] / scene_rho[i];
        scene_v[i] = scene_rhov[i] / scene_rho[i];
        scene_p[i] = scene_solver.ideal_gas_law_p(scene_e[i], scene_rho[i], scene_u[i], scene_v[i]);
    }

    for (std::size_t i = 0; i < scene_mesh.get_cells_size().x; ++i)
    {
        ivec2 cell_position = { i, 1 };
        f32 x = (f32)i / (f32)scene_mesh.get_cells_size().x;
        std::size_t cell_index = scene_mesh.get_cell_index(cell_position);

        riemann_solver::w exact_state = analytical_solution.sample(x - 0.5, 0.2f);

        residuals.rho += abs(exact_state.rho - scene_rho[cell_index]) * dx;
        residuals.u += abs(exact_state.u - scene_u[cell_index]) * dx;
        residuals.p += abs(exact_state.p - scene_p[cell_index]) * dx;
    }

    return residuals;
}

void print_residual(std::string sim_name, const riemann_solver::w& residuals)
{
    std::cout << "\n" << sim_name << "\n";
    std::cout << "Rho Residual: " << residuals.rho << "\n";
    std::cout << "u Residual: " << residuals.u << "\n";
    std::cout << "Pressure Residual: " << residuals.p << "\n";
}

int main()
{
    riemann_solver::w sim_1250 = run_sod_shock("sim_1250", 1250);
    riemann_solver::w sim_2500 = run_sod_shock("sim_2500", 2500);
    riemann_solver::w sim_5000 = run_sod_shock("sim_5000", 5000);
    riemann_solver::w sim_10000 = run_sod_shock("sim_10000", 10000);
    riemann_solver::w sim_20000 = run_sod_shock("sim_20000", 20000);

    print_residual("sim_1250", sim_1250);
    print_residual("sim_2500", sim_2500);
    print_residual("sim_5000", sim_5000);
    print_residual("sim_10000", sim_10000);
    print_residual("sim_20000", sim_20000);

    return 0;
}
