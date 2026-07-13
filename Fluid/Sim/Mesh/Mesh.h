#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <span>
#include <string_view>
#include <string>
#include <functional>

#include "../BoundaryRegion/BoundaryRegion.h"

class Mesh
{
public:
    struct Config
    {
        ivec2 Resolution = ivec2(0, 0);
        vec2 Dimensions = vec2(0.0f, 0.0f);
    };

    // Constructors & Destructors
    Mesh();
    Mesh(const Config& MeshConfig);

    vec2 MeshDimensions() const;
    f32 Getdx() const;
    f32 Getdy() const;
    
    //Face Index Helpers
    bool IsValidFace(i32 FaceIndex) const;

    // Face Getters
    i32 GetLeftCell(i32 FaceIndex) const;
    i32 GetRightCell(i32 FaceIndex) const;
    vec2 GetNormal(i32 FaceIndex) const;
    vec2 GetTangent(i32 FaceIndex) const;

    // Face Raw Data Getters
    std::span<const i32> GetLeftCells() const;
    std::span<const i32> GetRightCells() const;
    std::span<const vec2> GetNormals() const;
    std::span<const vec2> GetTangents() const;
    i32 GetFacesSize() const;
    
    // Cell Index Helpers
    bool IsValidCell(i32 Index) const;
    bool IsValidCell(const ivec2& Position) const;
    i32 GetCellIndex(const ivec2& Position) const;
    ivec2 GetCellPosition(i32 Index) const;

    // Cell Getters
    i32 GetTopFace(i32 CellIdx) const;
    i32 GetBottomFace(i32 CellIdx) const;
    i32 GetLeftFace(i32 CellIdx) const;
    i32 GetRightFace(i32 CellIdx) const;
    i32 GetTopFace(const ivec2& CellPosition) const;
    i32 GetBottomFace(const ivec2& CellPosition) const;
    i32 GetLeftFace(const ivec2& CellPosition) const;
    i32 GetRightFace(const ivec2& CellPosition) const;

    // Cell Raw Data Getters
    std::span<const i32> GetTopFaceIdxs() const;
    std::span<const i32> GetBottomFaceIdxs() const;
    std::span<const i32> GetLeftFaceIdxs() const;
    std::span<const i32> GetRightFaceIdxs() const;
    ivec2 GetCellsSize() const;
    i32 GetCellsSizeFlat() const;


    // Boundary Info Getters
    std::span<const i32> GetFacesInBoundaryRegion(const std::string& RegionName) const;
    std::span<const i32> GetFacesInBoundaryRegion(i32 RegionIdx) const;
    std::string_view GetRegionName(i32 RegionIdx) const;
    i32 GetRegionIdx(const std::string& RegionName) const;
    std::span<const BoundaryRegion> GetBoundaryRegions() const;

    // Mesh Editors
    void SetVerticallyPeriodic();
    void SetHorizontallyPeriodic();

    void AddBoundaryRegion(const BoundaryRegion::BoundaryConfig& Config);
private:
    vec2 Dimensions = vec2(0.0f, 0.0f);
    f32 dy = 0.0f;
    f32 dx = 0.0f;

    struct FacesArray
    {
        std::vector<i32> LeftCell = {  };
        std::vector<i32> RightCell = {  };
        std::vector<vec2> Normal = {  };
        std::vector<vec2> Tangent = {  };

        i32 Size = 0;
    };

    FacesArray Faces = {  };
    
    struct CellsArray
    {
        std::vector<i32> TopFaceIdx = {  };
        std::vector<i32> BottomFaceIdx = {  };
        std::vector<i32> LeftFaceIdx = {  };
        std::vector<i32> RightFaceIdx = {  };

        ivec2 Size = ivec2(0, 0);
        i32 SizeFlat = 0;
    };

    CellsArray Cells = {  };

    std::vector<BoundaryRegion> BoundaryRegions = {  };
    std::unordered_map<std::string, i32> BoundaryRegionsMap = {  };
};