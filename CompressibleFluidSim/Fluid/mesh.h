#pragma once
//#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <span>
#include <string_view>
#include <string>
#include <functional>
#include <vector>

#include "types.h"
#include "boundary_region.h"

class mesh
{
public:
    struct config
    {
        ivec2 resolution = ivec2(0, 0);
        vec2 dimensions = vec2(0.0f, 0.0f);
        std::vector<boundary_region::boundary_config> region_configs;
        bool vertically_periodic = false;
        bool horizontally_periodic = false;
    };

    // Constructors & Destructors
    mesh();
    mesh(const config& mesh_config);

    vec2 mesh_dimensions() const;
    f32 get_dx() const;
    f32 get_dy() const;

    //Face Index Helpers
    bool is_valid_face(i32 face_index) const;

    // Face Getters
    i32 get_left_cell(i32 face_index) const;
    i32 get_right_cell(i32 face_index) const;
    vec2 get_normal(i32 face_index) const;
    vec2 get_tangent(i32 face_index) const;
    f32 get_dl(i32 face_index) const;
    f32 get_invdl(i32 face_index) const;

    // Face Raw Data Getters
    std::span<const i32> get_left_cells() const;
    std::span<const i32> get_right_cells() const;
    std::span<const vec2> get_normals() const;
    std::span<const vec2> get_tangents() const;
    std::span<const f32> get_dls() const;
    std::span<const f32> get_invdls() const;

    i32 get_faces_size() const;

    // Cell Index Helpers
    bool is_valid_cell(i32 index) const;
    bool is_valid_cell(const ivec2& position) const;
    i32 get_cell_index(const ivec2& position) const;
    ivec2 get_cell_position(i32 index) const;

    // Cell Getters
    i32 get_top_face(i32 cell_idx) const;
    i32 get_bottom_face(i32 cell_idx) const;
    i32 get_left_face(i32 cell_idx) const;
    i32 get_right_face(i32 cell_idx) const;
    i32 get_top_face(const ivec2& cell_position) const;
    i32 get_bottom_face(const ivec2& cell_position) const;
    i32 get_left_face(const ivec2& cell_position) const;
    i32 get_right_face(const ivec2& cell_position) const;

    // Cell Raw Data Getters
    std::span<const i32> get_top_face_idxs() const;
    std::span<const i32> get_bottom_face_idxs() const;
    std::span<const i32> get_left_face_idxs() const;
    std::span<const i32> get_right_face_idxs() const;
    ivec2 get_cells_size() const;
    i32 get_cells_size_flat() const;


    // Boundary Info Getters
    std::string_view get_region_name(i32 region_idx) const;
    i32 get_region_idx(const std::string& region_name) const;
    std::span<const boundary_region> get_boundary_regions() const;

    // Mesh Editors
    void set_vertically_periodic();
    void set_horizontally_periodic();

    void add_boundary_region(const boundary_region::boundary_config& config);
private:
    vec2 dimensions = vec2(0.0f, 0.0f);
    f32 dy = 0.0f;
    f32 dx = 0.0f;

    struct faces_array
    {
        std::vector<i32> left_cell = {  };
        std::vector<i32> right_cell = {  };
        std::vector<vec2> normal = {  };
        std::vector<vec2> tangent = {  };
        std::vector<f32> dl = {  };
        std::vector<f32> invdl = {  };

        i32 size = 0;
    };

    faces_array faces = {  };

    struct cells_array
    {
        std::vector<i32> top_face_idx = {  };
        std::vector<i32> bottom_face_idx = {  };
        std::vector<i32> left_face_idx = {  };
        std::vector<i32> right_face_idx = {  };

        ivec2 size = ivec2(0, 0);
        i32 size_flat = 0;
    };

    cells_array cells = {  };

    std::vector<boundary_region> boundary_regions = {  };
    std::unordered_map<std::string, i32> boundary_regions_map = {  };
};