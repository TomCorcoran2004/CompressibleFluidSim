#include <iostream>

#include <mesh.h>
#include <boundary_region.h>
#include <solver.h>
#include <riemann_solver.h>
#include <mesh_structs.h>

bool IsFaceInEdgeRegion(const mesh_structs::face_info& face_info, const mesh_structs::mesh_info& mesh_info)
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

bool IsFaceInFluidRegion(const mesh_structs::face_info& face_info, const mesh_structs::mesh_info& mesh_info)
{
    return !IsFaceInEdgeRegion(face_info, mesh_info);
}

void run_sod_shock(std::string sim_name, std::size_t resolution)
{
    mesh_structs::boundary_config slip_wall = {
        .type = region_structs::types::slip_wall,
        .face_in_region = IsFaceInEdgeRegion,
    };

    mesh_structs::boundary_config fluid_region = {
        .type = region_structs::types::none,
        .face_in_region = IsFaceInFluidRegion,
    };

    mesh_structs::config mesh_config = {
        .resolution = {resolution, resolution},
        .dimensions = {1.0f, 1.0f},
        .vertically_periodic = false,
        .horizontally_periodic = false,
        .region_configs = {slip_wall, fluid_region}
    };

    solver::config solver_config = {
        .mesh_config = mesh_config,
        .r = 1.0f,
        .gamma = 1.4f,
        .cfl_target = 0.5f,
        .scene_time = 0.3f
    };

    solver scene_solver{ solver_config };

    const mesh& scene_mesh = scene_solver.get_mesh();

    for (std::size_t i = 0; i < scene_mesh.cells_size_flat(); ++i)
    {
        u64vec2 cell_mesh_position = scene_mesh.get_cell_position(i);
        u64vec2 cell_mesh_size = scene_mesh.cells_size();

        vec2 cell_position = {
            .x = (f32)cell_mesh_position.x / (f32)cell_mesh_size.x,
            .y = (f32)cell_mesh_position.y / (f32)cell_mesh_size.y,
        };

        if (cell_position.x < 0.5f)
        {
            if (cell_position.y < 0.5f) // bottom left
            {
                constexpr solver::primitive_state state = {
                    .rho = 0.1380f,
                    .u = 1.2060f,
                    .v = 1.2060f,
                    .p = 0.0290f
                };

                scene_solver.set_primitive_state(i, state);
            }
            else // top left
            {
                constexpr solver::primitive_state state = {
                    .rho = 0.5323f,
                    .u = 1.2060f,
                    .v = 0.0000f,
                    .p = 0.3000f
                };

                scene_solver.set_primitive_state(i, state);
            }
        }
        else
        {
            if (cell_position.y < 0.5f) // bottom right
            {
                constexpr solver::primitive_state state = {
                    .rho = 0.5323f,
                    .u = 0.0000f,
                    .v = 1.2060f,
                    .p = 0.3000f
                };

                scene_solver.set_primitive_state(i, state);
            }
            else // top right
            {
                constexpr solver::primitive_state state = {
                    .rho = 1.5000f,
                    .u = 0.0000f,
                    .v = 0.0000f,
                    .p = 1.5000f
                };

                scene_solver.set_primitive_state(i, state);
            }
        }
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
}

int main()
{
    run_sod_shock("sod_shock_125", 125);
    run_sod_shock("sod_shock_250", 250);
    run_sod_shock("sod_shock_500", 500);
    run_sod_shock("sod_shock_1000", 1000);
    //run_sod_shock("sod_shock_2000", 2000);
    return 0;
}
