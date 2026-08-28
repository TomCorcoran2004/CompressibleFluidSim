#include "mesh.h"
#include <malloc.h>
#include <vector>

mesh::mesh()
{
    faces = faces_array();
    cells = cells_array();
}

mesh::mesh(const config& mesh_config) : mesh()
{
    if (mesh_config.resolution.x < 3 || mesh_config.resolution.y < 3 || mesh_config.dimensions.x < 1e-6f || mesh_config.dimensions.y < 1e-6f)
    {
        return;
    }

    dimensions = mesh_config.dimensions;

    dx = dimensions.x / mesh_config.resolution.x;
    dy = dimensions.y / mesh_config.resolution.y;

    // Setting Up Cells Arrays
    cells.size = mesh_config.resolution;
    cells.size_flat = cells.size.x * cells.size.y;

    cells.top_face_idx.resize(cells.size_flat);
    cells.bottom_face_idx.resize(cells.size_flat);
    cells.left_face_idx.resize(cells.size_flat);
    cells.right_face_idx.resize(cells.size_flat);

    // Setting Up Faces Arrays
    ivec2 horizontal_faces_size = ivec2(cells.size.x, cells.size.y - 1);
    i32 horizontal_faces_size_flat = horizontal_faces_size.x * horizontal_faces_size.y;

    ivec2 vertical_faces_size = ivec2(cells.size.x - 1, cells.size.y);
    i32 vertical_faces_size_flat = vertical_faces_size.x * vertical_faces_size.y;

    faces.size = horizontal_faces_size_flat + vertical_faces_size_flat;

    faces.left_cell.resize(faces.size);
    faces.right_cell.resize(faces.size);
    faces.normal.resize(faces.size);
    faces.tangent.resize(faces.size);
    faces.dl.resize(faces.size);
    faces.invdl.resize(faces.size);

    // Setting Up Default Face Data
    for (i32 i = 0; i < horizontal_faces_size_flat; ++i)
    {
        ivec2 position;
        position.x = i % horizontal_faces_size.x;
        position.y = i / horizontal_faces_size.x;

        i32 left_cell_index = position.y * cells.size.x + position.x;
        i32 right_cell_index = left_cell_index + cells.size.x;

        faces.normal[i] = vec2(0.0f, 1.0f);
        faces.left_cell[i] = left_cell_index;
        faces.right_cell[i] = right_cell_index;
        
        vec2 normal = faces.normal[i];

        faces.tangent[i] = vec2(1.0f, 0.0f);
        faces.dl[i] = normal.x * dx + normal.y * dy;
        faces.invdl[i] = 1.0f / faces.dl[i];
    }

    for (i32 i = horizontal_faces_size_flat; i < faces.size; ++i)
    {
        ivec2 position;
        position.x = (i - horizontal_faces_size_flat) % vertical_faces_size.x;
        position.y = (i - horizontal_faces_size_flat) / vertical_faces_size.x;

        i32 left_cell_index = position.y * cells.size.x + position.x;
        i32 right_cell_index = left_cell_index + 1;


        faces.left_cell[i] = left_cell_index;
        faces.right_cell[i] = right_cell_index;
        faces.normal[i] = vec2(1.0f, 0.0f);
        
        vec2 normal = faces.normal[i];
        
        faces.tangent[i] = vec2(0.0f, 1.0f);
        faces.dl[i] = normal.x * dx + normal.y * dy;
        faces.invdl[i] = 1.0f / faces.dl[i];
    }

    // Setting Up Default Cell Data
    std::fill(cells.top_face_idx.begin(), cells.top_face_idx.end(), -1);
    std::fill(cells.bottom_face_idx.begin(), cells.bottom_face_idx.end(), -1);
    std::fill(cells.left_face_idx.begin(), cells.left_face_idx.end(), -1);
    std::fill(cells.right_face_idx.begin(), cells.right_face_idx.end(), -1);

    for (i32 i = 0; i < faces.size; ++i)
    {
        i32 left_cell = faces.left_cell[i];
        i32 right_cell = faces.right_cell[i];

        if (i < horizontal_faces_size_flat)
        {
            cells.bottom_face_idx[right_cell] = i;
            cells.top_face_idx[left_cell] = i;
        }
        else
        {
            cells.left_face_idx[right_cell] = i;
            cells.right_face_idx[left_cell] = i;
        }
    }
}

vec2 mesh::mesh_dimensions() const { return dimensions; }
f32 mesh::get_dx() const { return dx; }
f32 mesh::get_dy() const { return dy; }

bool mesh::is_valid_face(i32 face_index) const { return face_index >= 0 && face_index < faces.size; }

i32 mesh::get_left_cell(i32 face_index)  const { return faces.left_cell[face_index]; }
i32 mesh::get_right_cell(i32 face_index) const { return faces.right_cell[face_index]; }
vec2 mesh::get_normal(i32 face_index)   const { return faces.normal[face_index]; }
vec2 mesh::get_tangent(i32 face_index)  const { return faces.tangent[face_index]; }
f32 mesh::get_dl(i32 face_index)  const { return faces.dl[face_index]; }
f32 mesh::get_invdl(i32 face_index)  const { return faces.invdl[face_index]; }

std::span<const i32> mesh::get_left_cells()  const { return faces.left_cell; }
std::span<const i32> mesh::get_right_cells() const { return faces.right_cell; }
std::span<const vec2> mesh::get_normals()   const { return faces.normal; }
std::span<const vec2> mesh::get_tangents()  const { return faces.tangent; }
std::span<const f32> mesh::get_dls()  const { return faces.dl; }
std::span<const f32> mesh::get_invdls()  const { return faces.invdl; }
i32 mesh::get_faces_size() const { return faces.size; }

bool mesh::is_valid_cell(i32 index)             const { return index >= 0 && index < cells.size_flat; }
bool mesh::is_valid_cell(const ivec2& position) const { return position.x >= 0 && position.y >= 0 && position.x < cells.size.x && position.y < cells.size.y; }
i32 mesh::get_cell_index(const ivec2& position) const { return position.y * cells.size.x + position.x; }
ivec2 mesh::get_cell_position(i32 index)        const { return ivec2(index % cells.size.x, index / cells.size.x); }

i32 mesh::get_top_face(i32 cell_idx)    const { return cells.top_face_idx[cell_idx]; }
i32 mesh::get_bottom_face(i32 cell_idx) const { return cells.bottom_face_idx[cell_idx]; }
i32 mesh::get_left_face(i32 cell_idx)   const { return cells.left_face_idx[cell_idx]; }
i32 mesh::get_right_face(i32 cell_idx)  const { return cells.right_face_idx[cell_idx]; }
i32 mesh::get_top_face(const ivec2& cell_position)    const { return cells.top_face_idx[get_cell_index(cell_position)]; }
i32 mesh::get_bottom_face(const ivec2& cell_position) const { return cells.bottom_face_idx[get_cell_index(cell_position)]; }
i32 mesh::get_left_face(const ivec2& cell_position)   const { return cells.left_face_idx[get_cell_index(cell_position)]; }
i32 mesh::get_right_face(const ivec2& cell_position) const { return cells.right_face_idx[get_cell_index(cell_position)]; }

std::span<const i32> mesh::get_top_face_idxs()    const { return cells.top_face_idx; }
std::span<const i32> mesh::get_bottom_face_idxs() const { return cells.bottom_face_idx; }
std::span<const i32> mesh::get_left_face_idxs()   const { return cells.left_face_idx; }
std::span<const i32> mesh::get_right_face_idxs()  const { return cells.right_face_idx; }
ivec2 mesh::get_cells_size()   const { return cells.size; }
i32 mesh::get_cells_size_flat() const { return cells.size_flat; }

std::span<const boundary_region> mesh::get_boundary_regions() const { return boundary_regions; }

std::string_view mesh::get_region_name(i32 region_idx)        const { return boundary_regions[region_idx].get_name(); }
i32 mesh::get_region_idx(const std::string& region_name)      const { return boundary_regions_map.at(region_name); }

void mesh::set_vertically_periodic()
{
    i32 new_faces_size = faces.size + cells.size.x;

    faces.left_cell.resize(new_faces_size);
    faces.right_cell.resize(new_faces_size);
    faces.normal.resize(new_faces_size);
    faces.tangent.resize(new_faces_size);
    faces.dl.resize(new_faces_size);
    faces.invdl.resize(new_faces_size);

    for (i32 i = faces.size, x = 0; i < new_faces_size; ++i, ++x)
    {

        faces.left_cell[i] = get_cell_index(ivec2(x, cells.size.y - 1));
        faces.right_cell[i] = get_cell_index(ivec2(x, 0));
        faces.normal[i] = vec2(0.0f, 1.0f);
        faces.tangent[i] = vec2(1.0f, 0.f);
        
        vec2 normal = get_normal(i);
        faces.dl[i] = normal.x * dx + normal.y * dy;
        faces.invdl[i] = 1.0f / faces.dl[i];
    }

    for (i32 x = 0; x < cells.size.x; ++x)
    {
        i32 bottom_cell = get_cell_index(ivec2(x, 0));
        i32 top_cell = get_cell_index(ivec2(x, cells.size.y - 1));

        cells.bottom_face_idx[bottom_cell] = faces.size + x;
        cells.top_face_idx[top_cell] = faces.size + x;
    }

    faces.size = new_faces_size;
}

void mesh::set_horizontally_periodic()
{
    i32 new_faces_size = faces.size + cells.size.y;

    faces.left_cell.resize(new_faces_size);
    faces.right_cell.resize(new_faces_size);
    faces.normal.resize(new_faces_size);
    faces.tangent.resize(new_faces_size);
    faces.dl.resize(new_faces_size);
    faces.invdl.resize(new_faces_size);

    for (i32 i = faces.size, y = 0; i < new_faces_size; ++i, ++y)
    {
        
        faces.left_cell[i] = get_cell_index(ivec2(cells.size.x - 1, y));
        faces.right_cell[i] = get_cell_index(ivec2(0, y));
        faces.normal[i] = vec2(1.0f, 0.0f);
        faces.tangent[i] = vec2(0.0f, 1.f);
        
        vec2 normal = get_normal(i);
        faces.dl[i] = normal.x * dx + normal.y * dy;
        faces.invdl[i] = 1.0f / faces.dl[i];
    }

    for (i32 y = 0; y < cells.size.y; ++y)
    {
        i32 left_cell = get_cell_index(ivec2(0, y));
        i32 right_cell = get_cell_index(ivec2(cells.size.x - 1, y));

        cells.left_face_idx[left_cell] = faces.size + y;
        cells.right_face_idx[right_cell] = faces.size + y;
    }

    faces.size = new_faces_size;
}

void mesh::add_boundary_region(const boundary_region::boundary_config& config)
{
    i32 region_idx = boundary_regions.size();
    boundary_region& region = boundary_regions.emplace_back(config);

    boundary_regions_map[config.name] = region_idx; //TODO: Handle Same Name Errors

    for (i32 i = 0; i < faces.size; ++i)
    {
        boundary_region::face_info face_info = {
            .left_cell = get_left_cell(i),
            .right_cell = get_right_cell(i),
            .normal = get_normal(i)
        };

        boundary_region::mesh_info mesh_info = {
            .mesh_resolution = cells.size ,
            .mesh_dimensions = dimensions
        };

        if (region.func(face_info, mesh_info))
        {
            region.add_face(i);
        }
    }
}