#include "Mesh.h"
#include <malloc.h>
#include <vector>

Mesh::Mesh()
{
    Faces = FacesArray();
    Cells = CellsArray();
}

Mesh::Mesh(const Config& MeshConfig) : Mesh()
{
    if (MeshConfig.Resolution.x < 3 || MeshConfig.Resolution.y < 3 || MeshConfig.Dimensions.x < 1e-6f || MeshConfig.Dimensions.y < 1e-6f)
    {
        return;
    }

    Dimensions = MeshConfig.Dimensions;

    dx = Dimensions.x / MeshConfig.Resolution.x;
    dy = Dimensions.y / MeshConfig.Resolution.y;
    
    // Setting Up Cells Arrays
    Cells.Size = MeshConfig.Resolution;
    Cells.SizeFlat = Cells.Size.x * Cells.Size.y;
    
    Cells.TopFaceIdx.resize(Cells.SizeFlat);
    Cells.BottomFaceIdx.resize(Cells.SizeFlat);
    Cells.LeftFaceIdx.resize(Cells.SizeFlat);
    Cells.RightFaceIdx.resize(Cells.SizeFlat);

    // Setting Up Faces Arrays
    ivec2 HorizontalFacesSize = ivec2(Cells.Size.x, Cells.Size.y - 1);
    i32 HorizontalFacesSizeFlat = HorizontalFacesSize.x * HorizontalFacesSize.y;

    ivec2 VerticalFacesSize = ivec2(Cells.Size.x - 1, Cells.Size.y);
    i32 VerticalFacesSizeFlat = VerticalFacesSize.x * VerticalFacesSize.y;

    Faces.Size = HorizontalFacesSizeFlat + VerticalFacesSizeFlat;

    Faces.LeftCell.resize(Faces.Size);
    Faces.RightCell.resize(Faces.Size);
    Faces.Normal.resize(Faces.Size);
    Faces.Tangent.resize(Faces.Size);
    Faces.dl.resize(Faces.Size);
    Faces.Invdl.resize(Faces.Size);

    // Setting Up Default Face Data
    for (i32 i = 0; i < HorizontalFacesSizeFlat; ++i)
    {
        ivec2 Position;
        Position.x = i % HorizontalFacesSize.x;
        Position.y = i / HorizontalFacesSize.x;

        i32 LeftCellIndex = Position.y * Cells.Size.x + Position.x;
        i32 RightCellIndex = LeftCellIndex + Cells.Size.x;

        Faces.LeftCell[i] = LeftCellIndex;
        Faces.RightCell[i] = RightCellIndex;
        Faces.Normal[i] = vec2(0.0f, 1.0f);
        Faces.Tangent[i] = vec2(1.0f, 0.0f);
        Faces.dl[i] = glm::dot(Faces.Normal[i], vec2(dx, dy));
        Faces.Invdl[i] = 1.0f / Faces.dl[i];
    }

    for (i32 i = HorizontalFacesSizeFlat; i < Faces.Size; ++i)
    {
        ivec2 Position;
        Position.x = (i - HorizontalFacesSizeFlat) % VerticalFacesSize.x;
        Position.y = (i - HorizontalFacesSizeFlat) / VerticalFacesSize.x;

        i32 LeftCellIndex = Position.y * Cells.Size.x + Position.x;
        i32 RightCellIndex = LeftCellIndex + 1;

        Faces.LeftCell[i] = LeftCellIndex;
        Faces.RightCell[i] = RightCellIndex;
        Faces.Normal[i] = vec2(1.0f, 0.0f);
        Faces.Tangent[i] = vec2(0.0f, 1.0f);
        Faces.dl[i] = glm::dot(Faces.Normal[i], vec2(dx, dy));;
        Faces.Invdl[i] = 1.0f / Faces.dl[i];
    }

    // Setting Up Default Cell Data
    std::fill(Cells.TopFaceIdx.begin(), Cells.TopFaceIdx.end(), -1);
    std::fill(Cells.BottomFaceIdx.begin(), Cells.BottomFaceIdx.end(), -1);
    std::fill(Cells.LeftFaceIdx.begin(), Cells.LeftFaceIdx.end(), -1);
    std::fill(Cells.RightFaceIdx.begin(), Cells.RightFaceIdx.end(), -1);

    for (i32 i = 0; i < Faces.Size; ++i)
    {
        i32 LeftCell = Faces.LeftCell[i];
        i32 RightCell = Faces.RightCell[i];

        if (i < HorizontalFacesSizeFlat)
        {
            Cells.BottomFaceIdx[LeftCell] = i;
            Cells.TopFaceIdx[RightCell] = i;
        }
        else
        {
            Cells.LeftFaceIdx[RightCell] = i;
            Cells.RightFaceIdx[LeftCell] = i;
        }
    }
}

vec2 Mesh::MeshDimensions() const { return Dimensions; }
f32 Mesh::Getdx() const { return dx; }
f32 Mesh::Getdy() const { return dy; }

bool Mesh::IsValidFace(i32 FaceIndex) const { return FaceIndex >= 0 && FaceIndex < Faces.Size; }

i32 Mesh::GetLeftCell(i32 FaceIndex)  const { return Faces.LeftCell[FaceIndex]; }
i32 Mesh::GetRightCell(i32 FaceIndex) const { return Faces.RightCell[FaceIndex]; }
vec2 Mesh::GetNormal(i32 FaceIndex)   const { return Faces.Normal[FaceIndex]; }
vec2 Mesh::GetTangent(i32 FaceIndex)  const { return Faces.Tangent[FaceIndex]; }
f32 Mesh::Getdl(i32 FaceIndex)  const { return Faces.dl[FaceIndex]; }
f32 Mesh::GetInvdl(i32 FaceIndex)  const { return Faces.Invdl[FaceIndex]; }

std::span<const i32> Mesh::GetLeftCells()  const { return Faces.LeftCell; }
std::span<const i32> Mesh::GetRightCells() const { return Faces.RightCell; }
std::span<const vec2> Mesh::GetNormals()   const { return Faces.Normal; }
std::span<const vec2> Mesh::GetTangents()  const { return Faces.Tangent; }
std::span<const f32> Mesh::Getdls()  const { return Faces.dl; }
std::span<const f32> Mesh::GetInvdls()  const { return Faces.Invdl; }
i32 Mesh::GetFacesSize() const { return Faces.Size; }

bool Mesh::IsValidCell(i32 Index)             const { return Index >= 0 && Index < Cells.SizeFlat; }
bool Mesh::IsValidCell(const ivec2& Position) const { return Position.x >= 0 && Position.y >= 0 && Position.x < Cells.Size.x && Position.y < Cells.Size.y; }
i32 Mesh::GetCellIndex(const ivec2& Position) const { return Position.y * Cells.Size.x + Position.x; }
ivec2 Mesh::GetCellPosition(i32 Index)        const { return ivec2(Index % Cells.Size.x, Index / Cells.Size.x); }

i32 Mesh::GetTopFace(i32 CellIdx)    const { return Cells.TopFaceIdx[CellIdx]; }
i32 Mesh::GetBottomFace(i32 CellIdx) const { return Cells.BottomFaceIdx[CellIdx]; }
i32 Mesh::GetLeftFace(i32 CellIdx)   const { return Cells.LeftFaceIdx[CellIdx]; }
i32 Mesh::GetRightFace(i32 CellIdx)  const { return Cells.RightFaceIdx[CellIdx]; }
i32 Mesh::GetTopFace(const ivec2& CellPosition)    const { return Cells.TopFaceIdx[GetCellIndex(CellPosition)]; }
i32 Mesh::GetBottomFace(const ivec2& CellPosition) const { return Cells.BottomFaceIdx[GetCellIndex(CellPosition)]; }
i32 Mesh::GetLeftFace(const ivec2& CellPosition)   const { return Cells.LeftFaceIdx[GetCellIndex(CellPosition)]; }
i32 Mesh::GetRightFace(const ivec2 & CellPosition) const { return Cells.RightFaceIdx[GetCellIndex(CellPosition)]; }

std::span<const i32> Mesh::GetTopFaceIdxs()    const { return Cells.TopFaceIdx; }
std::span<const i32> Mesh::GetBottomFaceIdxs() const { return Cells.BottomFaceIdx; }
std::span<const i32> Mesh::GetLeftFaceIdxs()   const { return Cells.LeftFaceIdx; }
std::span<const i32> Mesh::GetRightFaceIdxs()  const { return Cells.RightFaceIdx; }
ivec2 Mesh::GetCellsSize()   const { return Cells.Size; }
i32 Mesh::GetCellsSizeFlat() const { return Cells.SizeFlat; }

std::span<const BoundaryRegion> Mesh::GetBoundaryRegions() const { return BoundaryRegions; }
std::span<const i32> Mesh::GetFacesInBoundaryRegion(const std::string& RegionName) const  { return BoundaryRegions[GetRegionIdx(RegionName)].GetFaces(); }
std::span<const i32> Mesh::GetFacesInBoundaryRegion(i32 RegionIdx)                 const { return BoundaryRegions[RegionIdx].GetFaces(); }

std::string_view Mesh::GetRegionName(i32 RegionIdx)        const { return BoundaryRegions[RegionIdx].GetName(); }
i32 Mesh::GetRegionIdx(const std::string& RegionName)      const { return BoundaryRegionsMap.at(RegionName); }

void Mesh::SetVerticallyPeriodic()
{
    i32 NewFacesSize = Faces.Size + Cells.Size.x;

    Faces.LeftCell.resize(NewFacesSize);
    Faces.RightCell.resize(NewFacesSize);
    Faces.Normal.resize(NewFacesSize);
    Faces.Tangent.resize(NewFacesSize);

    for (i32 i = Faces.Size, x = 0; i < NewFacesSize; ++i, ++x)
    {
        Faces.LeftCell[i] = GetCellIndex(ivec2(x, Cells.Size.y - 1));
        Faces.RightCell[i] = GetCellIndex(ivec2(x, 0));
        Faces.Normal[i] = vec2(0.0f, 1.0f);
        Faces.Tangent[i] = vec2(1.0f, 0.f);
    }

    for (i32 x = 0; x < Cells.Size.x; ++x)
    {
        i32 BottomCell = GetCellIndex(ivec2(x, 0));
        i32 TopCell = GetCellIndex(ivec2(x, Cells.Size.y - 1));

        Cells.BottomFaceIdx[BottomCell] = Faces.Size + x;
        Cells.TopFaceIdx[TopCell] = Faces.Size + x;
    }

    Faces.Size = NewFacesSize;
}

void Mesh::SetHorizontallyPeriodic()
{
    i32 NewFacesSize = Faces.Size + Cells.Size.y;

    Faces.LeftCell.resize(NewFacesSize);
    Faces.RightCell.resize(NewFacesSize);
    Faces.Normal.resize(NewFacesSize);
    Faces.Tangent.resize(NewFacesSize);

    for (i32 i = Faces.Size, y = 0; i < NewFacesSize; ++i, ++y)
    {
        Faces.LeftCell[i] = GetCellIndex(ivec2(Cells.Size.x - 1, y));
        Faces.RightCell[i] = GetCellIndex(ivec2(0, y));
        Faces.Normal[i] = vec2(1.0f, 0.0f);
        Faces.Tangent[i] = vec2(0.0f, 1.f);
    }

    for (i32 y = 0; y < Cells.Size.y; ++y)
    {
        i32 LeftCell = GetCellIndex(ivec2(0, y));
        i32 RightCell = GetCellIndex(ivec2(Cells.Size.x - 1, y));

        Cells.LeftFaceIdx[LeftCell] = Faces.Size + y;
        Cells.RightFaceIdx[RightCell] = Faces.Size + y;
    }

    Faces.Size = NewFacesSize;
}

void Mesh::AddBoundaryRegion(const BoundaryRegion::BoundaryConfig& Config)
{
    i32 RegionIdx = BoundaryRegions.size();
    BoundaryRegion& Region = BoundaryRegions.emplace_back(Config);

    BoundaryRegionsMap[Config.Name] = RegionIdx; //TODO: Handle Same Name Errors

    for (i32 i = 0; i < Faces.Size; ++i)
    {
        BoundaryRegion::FaceInfo FaceInfo = {
            .LeftCell = GetLeftCell(i),
            .RightCell = GetRightCell(i),
            .Normal = GetNormal(i)
        };

        BoundaryRegion::MeshInfo MeshInfo = {
            .MeshResolution = Cells.Size ,
            .MeshDimensions = Dimensions
        };
                
        if (Region.Func(FaceInfo, MeshInfo))
        {
            Region.AddFace(i);
        }
    }
}