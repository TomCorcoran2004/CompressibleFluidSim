#pragma once
#include <string>
#include <vector>
#include <functional>
#include <span>

#include "types.h"

class boundary_region
{
public:

    enum class boundary_types : i32
    {
        none,
        slip_wall,
        supersonic_inflow,
        subsonic_inflow,
        supersonic_outflow,
        subsonic_outflow,
    };

    struct face_info
    {
        i32 left_cell;
        i32 right_cell;
        vec2 normal;
    };

    struct mesh_info
    {
        ivec2 mesh_resolution;
        vec2 mesh_dimensions;
    };

    using is_face_in_region = std::function<bool(const face_info&, const mesh_info&)>;
    const is_face_in_region func;

    struct boundary_config
    {
        std::string name;
        boundary_types type;
        is_face_in_region face_in_region;
    };

    boundary_region();
    boundary_region(const boundary_config& config);

    void add_face(i32 face_idx);

    std::string_view get_name() const;
    std::span<const i32> get_faces() const;
    boundary_types get_type() const;
private:
    std::string name;
    std::vector<i32> faces;
    boundary_types type;
};