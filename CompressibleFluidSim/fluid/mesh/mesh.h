#pragma once
#include <vector>
#include <span>
#include <functional>

#include "CompressibleFluidSim/fluid/mesh/mesh_structs.h"
#include "CompressibleFluidSim/fluid/utils/types.h"
#include "CompressibleFluidSim/fluid/mesh/boundary_regions/boundary_region.h"

class mesh
{
public:
    // Constructors & Destructors
    mesh();
    mesh(const mesh_structs::config& mesh_config);

    vec2 mesh_dimensions() const;
    f32 get_dx() const;
    f32 get_dy() const;

    //Face Index Helpers
    bool is_valid_face(std::size_t face_index) const;

    // Face Getters
    std::size_t get_left_cell(std::size_t face_index) const;
    std::size_t get_right_cell(std::size_t face_index) const;
    std::size_t get_region_id(std::size_t face_index) const;
    vec2 get_normal(std::size_t face_index) const;
    vec2 get_tangent(std::size_t face_index) const;
    f32 get_dl(std::size_t face_index) const;
    f32 get_inv_dl(std::size_t face_index) const;

    // Face Raw Data Getters
    std::span<const std::size_t> host_left_cells() const;
    std::span<const std::size_t> host_right_cells() const;
    std::span<const std::size_t> host_region_ids() const;
    std::span<const vec2> host_normals() const;
    std::span<const vec2> host_tangents() const;
    std::span<const f32> host_dls() const;
    std::span<const f32> host_inv_dls() const;

    const std::size_t* device_left_cells() const;
    const std::size_t* device_right_cells() const;
    const std::size_t* device_region_ids() const;
    const vec2* device_normals() const;
    const vec2* device_tangents() const;
    const f32* device_dls() const;
    const f32* device_inv_dls() const;

    std::size_t faces_size() const;

    // Cell Index Helpers
    bool is_valid_cell(std::size_t index) const;
    bool is_valid_cell(const u64vec2& position) const;
    std::size_t get_cell_index(const u64vec2& position) const;
    u64vec2 get_cell_position(std::size_t index) const;

    // Cell Getters
    std::size_t get_top_face(std::size_t cell_idx) const;
    std::size_t get_bottom_face(std::size_t cell_idx) const;
    std::size_t get_left_face(std::size_t cell_idx) const;
    std::size_t get_right_face(std::size_t cell_idx) const;
    std::size_t get_top_face(const u64vec2& cell_position) const;
    std::size_t get_bottom_face(const u64vec2& cell_position) const;
    std::size_t get_left_face(const u64vec2& cell_position) const;
    std::size_t get_right_face(const u64vec2& cell_position) const;

    // Cell Raw Data Getters
    std::span<const std::size_t> host_top_face_idxs() const;
    std::span<const std::size_t> host_bottom_face_idxs() const;
    std::span<const std::size_t> host_left_face_idxs() const;
    std::span<const std::size_t> host_right_face_idxs() const;

    const std::size_t* device_top_face_idxs() const;
    const std::size_t* device_bottom_face_idxs() const;
    const std::size_t* device_left_face_idxs() const;
    const std::size_t* device_right_face_idxs() const;

    u64vec2 cells_size() const;
    std::size_t cells_size_flat() const;

    std::span<const boundary_region> boundary_regions() const;
private:
    void set_vertically_periodic();
    void set_horizontally_periodic();
    void add_boundary_region(const mesh_structs::boundary_config& config);
        
    vec2 dimensions = vec2(0.0f, 0.0f);
    f32 dy = 0.0f;
    f32 dx = 0.0f;

    mesh_structs::host_faces_array host_faces = {  };
    mesh_structs::host_cells_array host_cells = {  };

    mesh_structs::device_faces_array device_faces = {  };
    mesh_structs::device_cells_array device_cells = {  };

    std::vector<boundary_region> _boundary_regions = {  };
};