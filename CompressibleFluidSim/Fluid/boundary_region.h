#pragma once
#include <string>
#include <vector>
#include <functional>
#include <span>

#include "types.h"

class boundary_region
{
public:

    using idx = std::size_t;
    using mesh_idx = std::size_t;
    using region_idx = std::size_t;

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

    void add_face(mesh_idx face_idx);

    std::string_view get_name() const;
    
    std::span<const mesh_idx> get_mesh_idxs() const;
    std::span<const region_idx> get_region_idxs() const;
    
    bool contains_face(mesh_idx face_idx) const;

    boundary_types get_type() const;
private:
    static constexpr idx invalid_idx = std::numeric_limits<idx>::max();
    std::string name;
    
    std::vector<mesh_idx> mesh_idxs;
    std::vector<region_idx> region_idxs;
    
    boundary_types type;
};