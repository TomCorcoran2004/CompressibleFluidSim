#pragma once
#include <string>
#include <vector>
#include <functional>
#include <span>

#include <glm/glm.hpp>

class BoundaryRegion
{
public:
    
    enum class BoundaryTypes : i32
    {
        None,
        SlipWall,
        SupersonicInflow,
        SubsonicInflow,
        SupersonicOutflow,
        SubsonicOutflow,
    };

    struct FaceInfo
    {
        i32 LeftCell;
        i32 RightCell;
        vec2 Normal;
    };

    struct MeshInfo
    {
        ivec2 MeshResolution;
        vec2 MeshDimensions;
    };

    using IsFaceInRegion = std::function<bool(const FaceInfo&, const MeshInfo&)>;
    const IsFaceInRegion Func;
     
    struct BoundaryConfig
    {
        std::string Name;
        BoundaryTypes Type; 
        IsFaceInRegion FaceInRegion;
    };

    BoundaryRegion();
    BoundaryRegion(const BoundaryConfig& Config);

    void AddFace(i32 Faceidx);

    std::string_view GetName() const;
    std::span<const i32> GetFaces() const;
    BoundaryTypes GetType() const;
private:
    std::string Name;
    std::vector<i32> Faces;
    BoundaryTypes Type;
};

