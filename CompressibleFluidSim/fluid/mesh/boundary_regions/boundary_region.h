#pragma once
#include <vector>
#include <span>
#include <unordered_set>

#include "CompressibleFluidSim/fluid/utils/types.h"
#include "CompressibleFluidSim/fluid/mesh/mesh_structs.h"
#include "CompressibleFluidSim/fluid/mesh/boundary_regions/region_structs.h"
#include "CompressibleFluidSim/fluid/utils/gpu_buffer.h"

class boundary_region
{
public:
    boundary_region();
    boundary_region(const mesh_structs::boundary_config& config);

    void add_face(std::size_t face_idx, std::size_t left_cell_idx, std::size_t right_cell_idx);

    void upload_to_device();
    
    std::span<const std::size_t> host_face_idxs() const;
    const std::unordered_set<std::size_t>& host_cell_idxs() const;

    const std::size_t* device_face_idxs() const;
    const std::size_t* device_cell_idxs() const;

    std::size_t num_faces() const;
    std::size_t num_cells() const;

    region_structs::types type() const;
    std::size_t id() const;
private:
    static std::size_t regions_count;
    
    std::size_t _id;
    region_structs::types _type;

    bool uploaded_to_device;

    std::vector<std::size_t> _host_face_idxs;
    std::unordered_set<std::size_t> _host_cell_idxs;

    gpu_buffer<std::size_t> _device_face_idxs;
    gpu_buffer<std::size_t> _device_cell_idxs;

    std::size_t generate_unique_id();
};