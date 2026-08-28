#include "boundary_region.h"

boundary_region::boundary_region()
    : name(""),
    func([](const face_info&, const mesh_info&) { return false; }),
    type(boundary_types::none)
{
    mesh_idxs = {};
    region_idxs = {};
}

boundary_region::boundary_region(const boundary_config& config)
    : name(config.name),
    func(config.face_in_region),
    type(config.type)
{
    mesh_idxs = {  };
    region_idxs = {  };
}

void boundary_region::add_face(mesh_idx face_idx)
{
    if (face_idx >= region_idxs.size())
    {
        region_idxs.resize(face_idx + 1, invalid_idx);
    }
    region_idxs[face_idx] = mesh_idxs.size();
    
    mesh_idxs.push_back(face_idx);
}

bool boundary_region::contains_face(mesh_idx face_idx) const
{
    if (face_idx >= region_idxs.size())
        return false;

    region_idx idx = region_idxs[face_idx];

    if (idx == invalid_idx)
        return false;

    return true;
}

std::string_view boundary_region::get_name() const
{
    return name;
}

std::span<const boundary_region::mesh_idx> boundary_region::get_mesh_idxs() const
{
    return mesh_idxs;
}

std::span<const boundary_region::region_idx> boundary_region::get_region_idxs() const
{
    return region_idxs;
}

boundary_region::boundary_types boundary_region::get_type() const
{
    return type;
}