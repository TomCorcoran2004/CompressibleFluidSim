#pragma once
#include <vector>
#include <functional>
#include "CompressibleFluidSim/fluid/utils/types.h"
#include "CompressibleFluidSim/fluid/mesh/boundary_regions/region_structs.h"
#include "CompressibleFluidSim/fluid/utils/gpu_buffer.h"

namespace mesh_structs
{
    struct face_info
    {
        std::size_t left_cell;
        std::size_t right_cell;
        vec2 normal;
    };

    struct mesh_info
    {
        u64vec2 mesh_resolution;
        vec2 mesh_dimensions;
    };
    
    using is_face_in_region = std::function<bool(const face_info&, const mesh_info&)>;
    
    struct boundary_config
    {
        region_structs::types type;
        is_face_in_region face_in_region;
    };
    
    struct config
    {
        u64vec2 resolution = u64vec2(0, 0);
        vec2 dimensions = vec2(0.0f, 0.0f);
        bool vertically_periodic = false;
        bool horizontally_periodic = false;
        std::vector<boundary_config> region_configs;
    };

    struct host_faces_array
    {
        std::vector<std::size_t> left_cell = {  };
        std::vector<std::size_t> right_cell = {  };
        std::vector<vec2> normal = {  };
        std::vector<vec2> tangent = {  };
        std::vector<f32> dl = {  };
        std::vector<f32> inv_dl = {  };
        std::vector<std::size_t> region_ids = {  };

        std::size_t size = 0;
    };

    struct host_cells_array
    {
        std::vector<std::size_t> top_face_idx = {  };
        std::vector<std::size_t> bottom_face_idx = {  };
        std::vector<std::size_t> left_face_idx = {  };
        std::vector<std::size_t> right_face_idx = {  };

        u64vec2 size = u64vec2(0, 0);
        std::size_t size_flat = 0;
    };

    struct device_faces_array
    {
        gpu_buffer<std::size_t> left_cell;
        gpu_buffer<std::size_t> right_cell;
        gpu_buffer<vec2> normal;
        gpu_buffer<vec2> tangent;
        gpu_buffer<f32> dl;
        gpu_buffer<f32> inv_dl;
        gpu_buffer<std::size_t> region_ids;
    };

    struct device_cells_array
    {
        gpu_buffer<std::size_t> top_face_idx;
        gpu_buffer<std::size_t> bottom_face_idx;
        gpu_buffer<std::size_t> left_face_idx;
        gpu_buffer<std::size_t> right_face_idx;
    };

};