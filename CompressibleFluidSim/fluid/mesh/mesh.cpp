#include "CompressibleFluidSim/fluid/mesh/mesh.h"
#include <vector>
#include <cassert>

using namespace mesh_structs;

mesh::mesh()
{
    host_faces = host_faces_array();
    host_cells = host_cells_array();
}

mesh::mesh(const config& mesh_config)
{
    assert(mesh_config.resolution.x >= 3 && "Cell Resolution Must Be Greater Than 3");
    assert(mesh_config.resolution.y >= 3 && "Cell Resolution Must Be Greater Than 3");
    assert(mesh_config.dimensions.x > 1e-6f && "Mesh Dimensions Must Be Greater Than 1e-6f");
    assert(mesh_config.dimensions.y > 1e-6f && "Mesh Dimensions Must Be Greater Than 1e-6f");

    dimensions = mesh_config.dimensions;

    dx = dimensions.x / mesh_config.resolution.x;
    dy = dimensions.y / mesh_config.resolution.y;

    // Setting Up Cells Arrays
    host_cells.size = mesh_config.resolution;
    host_cells.size_flat = host_cells.size.x * host_cells.size.y;

    host_cells.top_face_idx.resize(host_cells.size_flat);
    host_cells.bottom_face_idx.resize(host_cells.size_flat);
    host_cells.left_face_idx.resize(host_cells.size_flat);
    host_cells.right_face_idx.resize(host_cells.size_flat);

    // Setting Up Faces Arrays
    u64vec2 horizontal_faces_size = u64vec2(host_cells.size.x, host_cells.size.y - 1);
    std::size_t horizontal_faces_size_flat = horizontal_faces_size.x * horizontal_faces_size.y;

    u64vec2 vertical_faces_size = u64vec2(host_cells.size.x - 1, host_cells.size.y);
    std::size_t vertical_faces_size_flat = vertical_faces_size.x * vertical_faces_size.y;

    host_faces.size = horizontal_faces_size_flat + vertical_faces_size_flat;

    host_faces.left_cell.resize(host_faces.size);
    host_faces.right_cell.resize(host_faces.size);
    host_faces.region_ids.resize(host_faces.size);
    host_faces.normal.resize(host_faces.size);
    host_faces.tangent.resize(host_faces.size);
    host_faces.dl.resize(host_faces.size);
    host_faces.inv_dl.resize(host_faces.size);

    // Setting Up Default Face Data
    for (std::size_t i = 0; i < horizontal_faces_size_flat; ++i)
    {
        u64vec2 position = {
            .x = i % horizontal_faces_size.x,
            .y = i / horizontal_faces_size.x
        };

        std::size_t left_cell_index = position.y * host_cells.size.x + position.x;
        std::size_t right_cell_index = left_cell_index + host_cells.size.x;

        host_faces.normal[i] = vec2(0.0f, 1.0f);
        host_faces.left_cell[i] = left_cell_index;
        host_faces.right_cell[i] = right_cell_index;
        
        host_faces.tangent[i] = vec2(1.0f, 0.0f);
        host_faces.dl[i] = host_faces.normal[i].x * dx + host_faces.normal[i].y * dy;
        host_faces.inv_dl[i] = 1.0f / host_faces.dl[i];
    }

    for (std::size_t i = horizontal_faces_size_flat; i < host_faces.size; ++i)
    {
        u64vec2 position = {
            .x = (i - horizontal_faces_size_flat) % vertical_faces_size.x,
            .y = (i - horizontal_faces_size_flat) / vertical_faces_size.x,
        };

        std::size_t left_cell_index = position.y * host_cells.size.x + position.x;
        std::size_t right_cell_index = left_cell_index + 1;

        host_faces.left_cell[i] = left_cell_index;
        host_faces.right_cell[i] = right_cell_index;
        host_faces.normal[i] = vec2(1.0f, 0.0f);
        
        host_faces.tangent[i] = vec2(0.0f, 1.0f);
        host_faces.dl[i] = host_faces.normal[i].x * dx + host_faces.normal[i].y * dy;
        host_faces.inv_dl[i] = 1.0f / host_faces.dl[i];
    }

    // Setting Up Default Cell Data
    std::fill(host_cells.top_face_idx.begin(), host_cells.top_face_idx.end(), -1);
    std::fill(host_cells.bottom_face_idx.begin(), host_cells.bottom_face_idx.end(), -1);
    std::fill(host_cells.left_face_idx.begin(), host_cells.left_face_idx.end(), -1);
    std::fill(host_cells.right_face_idx.begin(), host_cells.right_face_idx.end(), -1);

    for (std::size_t i = 0; i < host_faces.size; ++i)
    {
        std::size_t left_cell = host_faces.left_cell[i];
        std::size_t right_cell = host_faces.right_cell[i];

        if (i < horizontal_faces_size_flat)
        {
            host_cells.bottom_face_idx[right_cell] = i;
            host_cells.top_face_idx[left_cell] = i;
        }
        else
        {
            host_cells.left_face_idx[right_cell] = i;
            host_cells.right_face_idx[left_cell] = i;
        }
    }

    if (mesh_config.horizontally_periodic) 
        set_horizontally_periodic();

    if (mesh_config.vertically_periodic) 
        set_vertically_periodic();
    
    for (const boundary_config& region : mesh_config.region_configs)
    {
        add_boundary_region(region);
    }

    device_faces.left_cell.upload(host_faces.left_cell.data(), host_faces.size);
    device_faces.right_cell.upload(host_faces.right_cell.data(), host_faces.size);
    device_faces.normal.upload(host_faces.normal.data(), host_faces.size);
    device_faces.tangent.upload(host_faces.tangent.data(), host_faces.size);
    device_faces.dl.upload(host_faces.dl.data(), host_faces.size);
    device_faces.inv_dl.upload(host_faces.inv_dl.data(), host_faces.size);
    device_faces.region_ids.upload(host_faces.region_ids.data(), host_faces.size);

    device_cells.top_face_idx.upload(host_cells.top_face_idx.data(), host_cells.size_flat);
    device_cells.bottom_face_idx.upload(host_cells.bottom_face_idx.data(), host_cells.size_flat);
    device_cells.left_face_idx.upload(host_cells.left_face_idx.data(), host_cells.size_flat);
    device_cells.right_face_idx.upload(host_cells.right_face_idx.data(), host_cells.size_flat);

    for (boundary_region& region : _boundary_regions)
    {
        region.upload_to_device();
    }
}

vec2 mesh::mesh_dimensions() const { return dimensions; }
f32 mesh::get_dx() const { return dx; }
f32 mesh::get_dy() const { return dy; }

bool mesh::is_valid_face(std::size_t face_index) const { return face_index < host_faces.size; }

std::size_t mesh::get_left_cell(std::size_t face_index)  const { return host_faces.left_cell[face_index]; }
std::size_t mesh::get_right_cell(std::size_t face_index) const { return host_faces.right_cell[face_index]; }
std::size_t mesh::get_region_id(std::size_t face_index) const { return host_faces.region_ids[face_index]; }
vec2 mesh::get_normal(std::size_t face_index)   const { return host_faces.normal[face_index]; }
vec2 mesh::get_tangent(std::size_t face_index)  const { return host_faces.tangent[face_index]; }
f32 mesh::get_dl(std::size_t face_index)  const { return host_faces.dl[face_index]; }
f32 mesh::get_inv_dl(std::size_t face_index)  const { return host_faces.inv_dl[face_index]; }

std::span<const std::size_t> mesh::host_left_cells()  const { return host_faces.left_cell; }
std::span<const std::size_t> mesh::host_right_cells() const { return host_faces.right_cell; }
std::span<const std::size_t> mesh::host_region_ids() const { return host_faces.region_ids; }
std::span<const vec2> mesh::host_normals()   const { return host_faces.normal; }
std::span<const vec2> mesh::host_tangents()  const { return host_faces.tangent; }
std::span<const f32> mesh::host_dls()  const { return host_faces.dl; }
std::span<const f32> mesh::host_inv_dls()  const { return host_faces.inv_dl; }

const std::size_t* mesh::device_left_cells()  const { return device_faces.left_cell.data(); }
const std::size_t* mesh::device_right_cells() const { return device_faces.right_cell.data(); }
const std::size_t* mesh::device_region_ids() const { return device_faces.region_ids.data(); }
const vec2* mesh::device_normals()   const { return device_faces.normal.data(); }
const vec2* mesh::device_tangents()  const { return device_faces.tangent.data(); }
const f32* mesh::device_dls()  const { return device_faces.dl.data(); }
const f32* mesh::device_inv_dls()  const { return device_faces.inv_dl.data(); }

std::size_t mesh::faces_size() const { return host_faces.size; }

bool mesh::is_valid_cell(std::size_t index)             const { return index < host_cells.size_flat; }
bool mesh::is_valid_cell(const u64vec2& position) const { return position.x < host_cells.size.x && position.y < host_cells.size.y; }
std::size_t mesh::get_cell_index(const u64vec2& position) const { return position.y * host_cells.size.x + position.x; }
u64vec2 mesh::get_cell_position(std::size_t index)        const { return u64vec2(index % host_cells.size.x, index / host_cells.size.x); }

std::size_t mesh::get_top_face(std::size_t cell_idx)    const { return host_cells.top_face_idx[cell_idx]; }
std::size_t mesh::get_bottom_face(std::size_t cell_idx) const { return host_cells.bottom_face_idx[cell_idx]; }
std::size_t mesh::get_left_face(std::size_t cell_idx)   const { return host_cells.left_face_idx[cell_idx]; }
std::size_t mesh::get_right_face(std::size_t cell_idx)  const { return host_cells.right_face_idx[cell_idx]; }
std::size_t mesh::get_top_face(const u64vec2& cell_position)    const { return host_cells.top_face_idx[get_cell_index(cell_position)]; }
std::size_t mesh::get_bottom_face(const u64vec2& cell_position) const { return host_cells.bottom_face_idx[get_cell_index(cell_position)]; }
std::size_t mesh::get_left_face(const u64vec2& cell_position)   const { return host_cells.left_face_idx[get_cell_index(cell_position)]; }
std::size_t mesh::get_right_face(const u64vec2& cell_position) const { return host_cells.right_face_idx[get_cell_index(cell_position)]; }

std::span<const std::size_t> mesh::host_top_face_idxs()    const { return host_cells.top_face_idx; }
std::span<const std::size_t> mesh::host_bottom_face_idxs() const { return host_cells.bottom_face_idx; }
std::span<const std::size_t> mesh::host_left_face_idxs()   const { return host_cells.left_face_idx; }
std::span<const std::size_t> mesh::host_right_face_idxs()  const { return host_cells.right_face_idx; }

const std::size_t* mesh::device_top_face_idxs()    const { return device_cells.top_face_idx.data(); }
const std::size_t* mesh::device_bottom_face_idxs() const { return device_cells.bottom_face_idx.data(); }
const std::size_t* mesh::device_left_face_idxs()   const { return device_cells.left_face_idx.data(); }
const std::size_t* mesh::device_right_face_idxs()  const { return device_cells.right_face_idx.data(); }

u64vec2 mesh::cells_size()   const { return host_cells.size; }
std::size_t mesh::cells_size_flat() const { return host_cells.size_flat; }

std::span<const boundary_region> mesh::boundary_regions() const { return _boundary_regions; }

void mesh::set_vertically_periodic()
{
    std::size_t new_faces_size = host_faces.size + host_cells.size.x;

    host_faces.left_cell.resize(new_faces_size);
    host_faces.right_cell.resize(new_faces_size);
    host_faces.normal.resize(new_faces_size);
    host_faces.tangent.resize(new_faces_size);
    host_faces.dl.resize(new_faces_size);
    host_faces.inv_dl.resize(new_faces_size);
    host_faces.region_ids.resize(new_faces_size);

    for (std::size_t i = host_faces.size, x = 0; i < new_faces_size; ++i, ++x)
    {

        host_faces.left_cell[i] = get_cell_index(u64vec2(x, host_cells.size.y - 1));
        host_faces.right_cell[i] = get_cell_index(u64vec2(x, 0));
        host_faces.normal[i] = vec2(0.0f, 1.0f);
        host_faces.tangent[i] = vec2(1.0f, 0.f);
        
        vec2 normal = get_normal(i);
        host_faces.dl[i] = normal.x * dx + normal.y * dy;
        host_faces.inv_dl[i] = 1.0f / host_faces.dl[i];
    }

    for (std::size_t x = 0; x < host_cells.size.x; ++x)
    {
        std::size_t bottom_cell = get_cell_index(u64vec2(x, 0));
        std::size_t top_cell = get_cell_index(u64vec2(x, host_cells.size.y - 1));

        host_cells.bottom_face_idx[bottom_cell] = host_faces.size + x;
        host_cells.top_face_idx[top_cell] = host_faces.size + x;
    }

    host_faces.size = new_faces_size;
}

void mesh::set_horizontally_periodic()
{
    std::size_t new_faces_size = host_faces.size + host_cells.size.y;

    host_faces.left_cell.resize(new_faces_size);
    host_faces.right_cell.resize(new_faces_size);
    host_faces.normal.resize(new_faces_size);
    host_faces.tangent.resize(new_faces_size);
    host_faces.dl.resize(new_faces_size);
    host_faces.inv_dl.resize(new_faces_size);
    host_faces.region_ids.resize(new_faces_size);

    for (std::size_t i = host_faces.size, y = 0; i < new_faces_size; ++i, ++y)
    {
        host_faces.left_cell[i] = get_cell_index(u64vec2(host_cells.size.x - 1, y));
        host_faces.right_cell[i] = get_cell_index(u64vec2(0, y));
        host_faces.normal[i] = vec2(1.0f, 0.0f);
        host_faces.tangent[i] = vec2(0.0f, 1.f);
        
        vec2 normal = get_normal(i);
        host_faces.dl[i] = normal.x * dx + normal.y * dy;
        host_faces.inv_dl[i] = 1.0f / host_faces.dl[i];
    }

    for (std::size_t y = 0; y < host_cells.size.y; ++y)
    {
        std::size_t left_cell = get_cell_index(u64vec2(0, y));
        std::size_t right_cell = get_cell_index(u64vec2(host_cells.size.x - 1, y));

        host_cells.left_face_idx[left_cell] = host_faces.size + y;
        host_cells.right_face_idx[right_cell] = host_faces.size + y;
    }

    host_faces.size = new_faces_size;
}

void mesh::add_boundary_region(const boundary_config& config)
{
    boundary_region& region = _boundary_regions.emplace_back(config);

    for (std::size_t i = 0; i < host_faces.size; ++i)
    {
        face_info face_info = {
            .left_cell = get_left_cell(i),
            .right_cell = get_right_cell(i),
            .normal = get_normal(i)
        };

        mesh_info _mesh_info = {
            .mesh_resolution = host_cells.size,
            .mesh_dimensions = dimensions
        };

        if (config.face_in_region(face_info, _mesh_info))
        {
            region.add_face(i, face_info.left_cell, face_info.right_cell);
            host_faces.region_ids[i] = region.id();
        }
    }
}