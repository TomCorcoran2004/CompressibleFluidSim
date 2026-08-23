#include "boundary_region.h"

boundary_region::boundary_region()
    : name(""),
    func([](const face_info&, const mesh_info&) { return false; }),
    type(boundary_types::none)
{
    faces = {  };
}

boundary_region::boundary_region(const boundary_config& config)
    : name(config.name),
    func(config.face_in_region),
    type(config.type)
{
    faces = {  };
}

void boundary_region::add_face(i32 face_idx)
{
    faces.push_back(face_idx);
}

std::string_view boundary_region::get_name() const
{
    return name;
}

std::span<const i32> boundary_region::get_faces() const
{
    return faces;
}

boundary_region::boundary_types boundary_region::get_type() const
{
    return type;
}