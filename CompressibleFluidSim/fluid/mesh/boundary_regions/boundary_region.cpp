#include "CompressibleFluidSim/fluid/mesh/boundary_regions/boundary_region.h"
#include <cassert>

std::size_t boundary_region::regions_count = 1;

boundary_region::boundary_region() :
    _type(region_structs::types::none),
    _id(generate_unique_id()),
    uploaded_to_device(false)
{
    _host_face_idxs = {  };
    _host_cell_idxs = {  };
}

boundary_region::boundary_region(const mesh_structs::boundary_config& config) :
    _type(config.type),
    _id(generate_unique_id()),
    uploaded_to_device(false)
{
    _host_face_idxs = {  };
    _host_cell_idxs = {  };
}

void boundary_region::add_face(std::size_t face_idx, std::size_t left_cell_idx, std::size_t right_cell_idx)
{
    _host_face_idxs.push_back(face_idx);

    //has to be 2 lines, or compiler thinks its 2 iterators and breaks everything
    _host_cell_idxs.insert(left_cell_idx);
    _host_cell_idxs.insert(right_cell_idx);
}

void boundary_region::upload_to_device()
{
    //convert set into contiguous buffer
    std::vector<std::size_t> temp = { _host_cell_idxs.begin(), _host_cell_idxs.end()};
    
    _device_cell_idxs.upload(temp.data(), temp.size());
    _device_face_idxs.upload(_host_face_idxs.data(), _host_face_idxs.size());

    uploaded_to_device = true;
}

std::span<const std::size_t> boundary_region::host_face_idxs() const
{
    return _host_face_idxs;
}

const std::unordered_set<std::size_t>& boundary_region::host_cell_idxs() const
{
    return _host_cell_idxs;
}

const std::size_t* boundary_region::device_face_idxs() const
{
    assert(uploaded_to_device && "Boundary Region Has Not Been Uploaded To Device");
    assert(_device_face_idxs.data());
    
    return _device_face_idxs.data();
}

const std::size_t* boundary_region::device_cell_idxs() const
{
    assert(uploaded_to_device && "Boundary Region Has Not Been Uploaded To Device");
    assert(_device_cell_idxs.data());

    return _device_cell_idxs.data();
}

std::size_t boundary_region::num_faces() const 
{ 
    return _host_face_idxs.size();
}

std::size_t boundary_region::num_cells() const 
{ 
    return _host_cell_idxs.size();
}

region_structs::types boundary_region::type() const
{
    return _type;
}

std::size_t boundary_region::id() const
{
    return _id;
}

std::size_t boundary_region::generate_unique_id()
{
    return ++regions_count;
}