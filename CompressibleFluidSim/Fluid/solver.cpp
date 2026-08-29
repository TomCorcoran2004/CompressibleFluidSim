#include "solver.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <span>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <stdexcept>


solver::solver(const config& config) : scene_mesh(std::move(mesh{config.mesh_config}))
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

void solver::write_vti_ascii(const std::string& filepath, const std::string& filename)
{
    using enum conserved_fields;
    using enum derived_fields;
    
    const std::size_t num_cels = scene_mesh.get_cells_size_flat();
    const ivec2 size = scene_mesh.get_cells_size();
    const f32 dx = scene_mesh.get_dx();
    const f32 dy = scene_mesh.get_dy();

    std::ofstream file(filepath + filename);

    if (!file)
    {
        throw std::runtime_error("Failed to open VTI file");
    }

    // Enough decimal digits to exactly round-trip a float.
    file << std::setprecision(std::numeric_limits<f32>::max_digits10);

    file << "<?xml version=\"1.0\"?>\n";
    file << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";

    file << "  <ImageData"
        << " WholeExtent=\"0 " << size.x
        << " 0 " << size.y
        << " 0 0\""
        << " Origin=\"0 0 0\""
        << " Spacing=\"" << dx << ' ' << dy << " 1\">\n";

    file << "    <Piece"
        << " Extent=\"0 " << size.x
        << " 0 " << size.y
        << " 0 0\">\n";

    file << "      <PointData/>\n";

    file << "      <CellData Scalars=\"rho\">\n";

    auto write_field = [&file, size](
        const char* name,
        std::span<const f32> data)
        {
            file << "        <DataArray"
                << " type=\"Float32\""
                << " Name=\"" << name << "\""
                << " NumberOfComponents=\"1\""
                << " format=\"ascii\">\n";

            file << "          ";

            for (std::size_t i = 0; i < data.size(); ++i)
            {
                file << data[i] << ' ';

                if ((i + 1) % size.x == 0 && i + 1 != data.size())
                {
                    file << '\n';
                    file << "          ";
                }
            }

            file << '\n';
            file << "        </DataArray>\n";
        };

    write_field("rho", conserved_states[rho]);
    write_field("rhou", conserved_states[rhou]);
    write_field("rhov", conserved_states[rhov]);
    write_field("e", conserved_states[e]);
    write_field("inv_rho", derived_states[inv_rho]);
    write_field("u", derived_states[u]);
    write_field("v", derived_states[v]);
    write_field("p", derived_states[p]);
    write_field("c", derived_states[c]);

    file << "      </CellData>\n";
    file << "    </Piece>\n";
    file << "  </ImageData>\n";
    file << "</VTKFile>\n";
}

void solver::write_vti_binary(const std::string& filepath, const std::string& filename)
{
    using enum conserved_fields;
    using enum derived_fields;

    const std::size_t num_cells = scene_mesh.get_cells_size_flat();
    const ivec2 size = scene_mesh.get_cells_size();
    const f32 dx = scene_mesh.get_dx();
    const f32 dy = scene_mesh.get_dy();

    std::ofstream file(filepath + filename, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("Failed to open VTI file");
    }

    struct field
    {
        const char* name;
        std::span<const f32> data;
    };

    const std::array<field, 9> fields = {
        field{"rho", conserved_states[rho]},
        field{"rhou", conserved_states[rhou]},
        field{"rhov", conserved_states[rhov]},
        field{"e", conserved_states[e]},
        field{"inv_rho", derived_states[inv_rho]},
        field{"u", derived_states[u]},
        field{"v", derived_states[v]},
        field{"p", derived_states[p]},
        field{"c", derived_states[c]}
    };

    file << "<?xml version=\"1.0\"?>\n";
    file << "<VTKFile"
        << " type=\"ImageData\""
        << " version=\"0.1\""
        << " byte_order=\"LittleEndian\""
        << " header_type=\"UInt64\">\n";

    file << "  <ImageData"
        << " WholeExtent=\"0 " << size.x
        << " 0 " << size.y
        << " 0 0\""
        << " Origin=\"0 0 0\""
        << " Spacing=\"" << dx << ' ' << dy << " 1\">\n";

    file << "    <Piece"
        << " Extent=\"0 " << size.x
        << " 0 " << size.y
        << " 0 0\">\n";

    file << "      <PointData/>\n";
    file << "      <CellData Scalars=\"rho\">\n";

    std::uint64_t offset = 0;

    for (const field& field : fields)
    {
        file << "        <DataArray"
            << " type=\"Float32\""
            << " Name=\"" << field.name << "\""
            << " NumberOfComponents=\"1\""
            << " format=\"appended\""
            << " offset=\"" << offset << "\"/>\n";

        const std::uint64_t data_size =
            static_cast<std::uint64_t>(field.data.size() * sizeof(f32));

        offset += sizeof(std::uint64_t) + data_size;
    }

    file << "      </CellData>\n";
    file << "    </Piece>\n";
    file << "  </ImageData>\n";

    file << "  <AppendedData encoding=\"raw\">\n_";

    for (const field& field : fields)
    {
        const std::uint64_t data_size =
            static_cast<std::uint64_t>(field.data.size() * sizeof(f32));

        file.write(
            reinterpret_cast<const char*>(&data_size),
            sizeof(data_size)
        );

        file.write(
            reinterpret_cast<const char*>(field.data.data()),
            static_cast<std::streamsize>(data_size)
        );
    }

    file << "\n  </AppendedData>\n";
    file << "</VTKFile>\n";
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

    derived_states[derived_fields::inv_rho][cell_idx] = inv_rho;
    derived_states[derived_fields::u][cell_idx] = state.u;
    derived_states[derived_fields::v][cell_idx] = state.v;
    derived_states[derived_fields::p][cell_idx] = state.p;
    derived_states[derived_fields::c][cell_idx] = c;
}

void solver::set_conserved_state(std::size_t cell_idx, const conserved_state& state)
{
    conserved_states[conserved_fields::rho][cell_idx] = state.rho;
    conserved_states[conserved_fields::rhou][cell_idx] = state.rhou;
    conserved_states[conserved_fields::rhov][cell_idx] = state.rhov;
    conserved_states[conserved_fields::e][cell_idx] = state.e;
    
    const f32 inv_rho = 1.0f / state.rho;
    const f32 u = state.rhou * inv_rho;
    const f32 v = state.rhov * inv_rho;
    const f32 p = ideal_gas_law_p(state.e, state.rho, u, v);

    
    derived_states[derived_fields::inv_rho][cell_idx] = inv_rho;
    derived_states[derived_fields::u][cell_idx] = u * inv_rho;
    derived_states[derived_fields::v][cell_idx] = v;
    derived_states[derived_fields::p][cell_idx] = p;
    derived_states[derived_fields::c][cell_idx] = std::sqrt(gamma * p * inv_rho);
}

solver::primitive_state solver::get_primitive_state(std::size_t cell_idx)
{
    using enum conserved_fields;
    using enum derived_fields;

    return primitive_state{
        .rho = conserved_states[rho][cell_idx],
        .u = derived_states[u][cell_idx],
        .v = derived_states[v][cell_idx],
        .p = derived_states[p][cell_idx],
    };
}

solver::conserved_state solver::get_conserved_state(std::size_t cell_idx)
{
    using enum conserved_fields;

    return conserved_state{
        .rho = conserved_states[rho][cell_idx],
        .rhou = conserved_states[rhou][cell_idx],
        .rhov = conserved_states[rhov][cell_idx],
        .e = conserved_states[e][cell_idx],
    };
}

void solver::set_rho(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.rho = val;

    set_primitive_state(cell_idx, state);
}

void solver::set_u(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.u = +val;

    set_primitive_state(cell_idx, state);
}

void solver::set_v(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.v = val;

    set_primitive_state(cell_idx, state);
}

void solver::set_p(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.p = val;

    set_primitive_state(cell_idx, state);
}

void solver::add_rho(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.rho += val;

    set_primitive_state(cell_idx, state);
}

void solver::add_u(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.u += val;

    set_primitive_state(cell_idx, state);
}

void solver::add_v(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.v += val;

    set_primitive_state(cell_idx, state);
}

void solver::add_p(std::size_t cell_idx, f32 val)
{
    primitive_state state = get_primitive_state(cell_idx);

    state.p += val;

    set_primitive_state(cell_idx, state);
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

const mesh& solver::get_mesh() const
{
    return scene_mesh;
}

void solver::calculate_derived_states()
{
    using enum conserved_fields;
    using enum derived_fields;

    auto range = std::views::iota(std::size_t(0), (std::size_t)scene_mesh.get_cells_size_flat());

    auto calculate_derived_state = [this](std::size_t i) -> void
    {
        derived_states[inv_rho][i] = 1.0f / conserved_states[rho][i];
        derived_states[v][i] = conserved_states[rhov][i] * derived_states[inv_rho][i];
        derived_states[u][i] = conserved_states[rhou][i] * derived_states[inv_rho][i];
        derived_states[p][i] = ideal_gas_law_p(conserved_states[e][i], conserved_states[rho][i], derived_states[u][i], derived_states[v][i]);
        derived_states[c][i] = std::sqrtf(gamma * derived_states[p][i] * derived_states[inv_rho][i]);
    };

    std::for_each(
        std::execution::par_unseq,
        range.begin(),
        range.end(),
        calculate_derived_state
    );
}

void solver::calculate_time_step()
{
    using enum derived_fields;

    auto range = std::views::iota(std::size_t(0), (std::size_t)scene_mesh.get_cells_size_flat());

    auto min = [](f32 a, f32 b)
    {
        return std::min(a, b);
    };

    auto calculate_min_time_step = [this](std::size_t i)
    {
        const f32 cfl_time_step_x = cfl_condition(scene_mesh.get_dx(), derived_states[u][i], derived_states[c][i]);
        const f32 cfl_time_step_y = cfl_condition(scene_mesh.get_dy(), derived_states[v][i], derived_states[c][i]);

        return std::min(cfl_time_step_x, cfl_time_step_y);
    };

    dt = std::transform_reduce(
        std::execution::par_unseq,
        range.begin(),
        range.end(),
        std::numeric_limits<f32>::max(),
        min,
        calculate_min_time_step
    );
}

void solver::calculate_face_flux_transfer()
{
    for (const boundary_region& region : scene_mesh.get_boundary_regions())
    {
        boundary_region::boundary_types type = region.get_type();

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

    const std::span<const boundary_region::mesh_idx> mesh_idxs = region.get_mesh_idxs();
    const std::span<const boundary_region::region_idx> region_idxs = region.get_region_idxs();
    
    if (fluxes.size() < mesh_idxs.size())
    {
        fluxes.resize(mesh_idxs.size());
    }

    auto mesh_idxs_range = std::views::iota(std::size_t(0), mesh_idxs.size());

    auto calculate_delta_flux = [this, &mesh_idxs](std::size_t i)
    {
        const i32 face_idx = mesh_idxs[i];
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

        fluxes[flux_fields::mass][i] = delta_flux[0] * dt_div_dl;
        fluxes[flux_fields::momentum_u][i] = delta_flux[1] * dt_div_dl;
        fluxes[flux_fields::momentum_v][i] = delta_flux[2] * dt_div_dl;
        fluxes[flux_fields::e][i] = delta_flux[3] * dt_div_dl;
    };

    std::for_each(
        std::execution::par_unseq,
        mesh_idxs_range.begin(),
        mesh_idxs_range.end(),
        calculate_delta_flux
    );

    auto cells_range = std::views::iota(std::size_t(0), (std::size_t)scene_mesh.get_cells_size_flat());

    auto apply_delta_flux = [this, region_idxs, region](std::size_t cell_idx)
    {
        ivec2 cell_position = scene_mesh.get_cell_position(cell_idx);
        ivec2 cells_size = scene_mesh.get_cells_size();
            
        i32 top_face = scene_mesh.get_top_face(cell_idx);
        if (scene_mesh.is_valid_face(top_face) && region.contains_face(top_face))
        {
            const boundary_region::region_idx top_region_idx = region_idxs[top_face];
            conserved_states_temp[rho][cell_idx] -= fluxes[flux_fields::mass][top_region_idx];
            conserved_states_temp[rhou][cell_idx] -= fluxes[flux_fields::momentum_u][top_region_idx];
            conserved_states_temp[rhov][cell_idx] -= fluxes[flux_fields::momentum_v][top_region_idx];
            conserved_states_temp[e][cell_idx] -= fluxes[flux_fields::e][top_region_idx];
        }
        
        i32 bottom_face = scene_mesh.get_bottom_face(cell_idx);
        if (scene_mesh.is_valid_face(bottom_face) && region.contains_face(bottom_face))
        {
            const boundary_region::region_idx bottom_region_idx = region_idxs[bottom_face];
            conserved_states_temp[rho][cell_idx] += fluxes[flux_fields::mass][bottom_region_idx];
            conserved_states_temp[rhou][cell_idx] += fluxes[flux_fields::momentum_u][bottom_region_idx];
            conserved_states_temp[rhov][cell_idx] += fluxes[flux_fields::momentum_v][bottom_region_idx];
            conserved_states_temp[e][cell_idx] += fluxes[flux_fields::e][bottom_region_idx];
        }
        
        i32 left_face = scene_mesh.get_left_face(cell_idx);
        if (scene_mesh.is_valid_face(left_face) && region.contains_face(left_face))
        {
            const boundary_region::region_idx left_region_idx = region_idxs[left_face];
            conserved_states_temp[rho][cell_idx] += fluxes[flux_fields::mass][left_region_idx];
            conserved_states_temp[rhou][cell_idx] += fluxes[flux_fields::momentum_u][left_region_idx];
            conserved_states_temp[rhov][cell_idx] += fluxes[flux_fields::momentum_v][left_region_idx];
            conserved_states_temp[e][cell_idx] += fluxes[flux_fields::e][left_region_idx];
        }
        
        i32 right_face = scene_mesh.get_right_face(cell_idx);
        if (scene_mesh.is_valid_face(right_face) && region.contains_face(right_face))
        {
            const boundary_region::region_idx right_region_idx = region_idxs[right_face];
            conserved_states_temp[rho][cell_idx] -= fluxes[flux_fields::mass][right_region_idx];
            conserved_states_temp[rhou][cell_idx] -= fluxes[flux_fields::momentum_u][right_region_idx];
            conserved_states_temp[rhov][cell_idx] -= fluxes[flux_fields::momentum_v][right_region_idx];
            conserved_states_temp[e][cell_idx] -= fluxes[flux_fields::e][right_region_idx];
        }
    };

    std::for_each(
        std::execution::par_unseq,
        cells_range.begin(),
        cells_range.end(),
        apply_delta_flux
    );
}

void solver::calculate_slip_wall_region(const boundary_region& region)
{
    using enum conserved_fields;
    using enum derived_fields;

    const std::size_t* __restrict faces = region.get_mesh_idxs().data();
    const i32* __restrict left_cells = scene_mesh.get_left_cells().data();
    const i32* __restrict right_cells = scene_mesh.get_right_cells().data();
    const vec2* __restrict normals = scene_mesh.get_normals().data();
    const f32* __restrict invdls = scene_mesh.get_invdls().data();

    const std::size_t size = region.get_mesh_idxs().size();

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