#include "BoundaryRegion.h"

BoundaryRegion::BoundaryRegion() 
    : Name(""), 
      Func([](const FaceInfo&, const MeshInfo&) { return false; }),
      Type(BoundaryTypes::None)
{
    Faces = {  };
}

BoundaryRegion::BoundaryRegion(const BoundaryConfig& Config)
    : Name(Config.Name), 
      Func(Config.FaceInRegion),
      Type(Config.Type)
{
    Faces = {  };
}

void BoundaryRegion::AddFace(i32 FaceIdx)
{
    Faces.push_back(FaceIdx);
}

std::string_view BoundaryRegion::GetName() const
{
    return Name;
}

std::span<const i32> BoundaryRegion::GetFaces() const 
{
    return Faces;
}

BoundaryRegion::BoundaryTypes BoundaryRegion::GetType() const
{
    return Type;
}