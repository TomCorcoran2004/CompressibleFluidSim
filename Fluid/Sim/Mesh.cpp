#include "Mesh.h"
#include <malloc.h>

Mesh::Mesh()
{
    Faces = FacesArray();
    Cells = CellsArray();
}

Mesh::Mesh(const MeshConfig& Config)
{
    // Setting Up Cells Arrays And Allocating Memory
    Cells.Size = Config.MeshDimensions;
    Cells.SizeFlat = Cells.Size.x * Cells.Size.y;
    
    Cells.Type = static_cast<CellType*>(_aligned_malloc(Cells.SizeFlat * sizeof(CellType), 64));
    Cells.TopFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    Cells.BottomFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    Cells.LeftFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    Cells.RightFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));

    // Setting Up Faces Arrays And Allocating Memory
    ivec2 HorizontalFacesSize = ivec2(Cells.Size.x, Cells.Size.y - 1);
    i32 HorizontalFacesSizeFlat = HorizontalFacesSize.x * HorizontalFacesSize.y;

    ivec2 VerticalFacesSize = ivec2(Cells.Size.x - 1, Cells.Size.y);
    i32 VerticalFacesSizeFlat = VerticalFacesSize.x * VerticalFacesSize.y;

    Faces.Size = HorizontalFacesSizeFlat + VerticalFacesSizeFlat;

    Faces.LeftCell = static_cast<i32*>(_aligned_malloc(Faces.Size * sizeof(i32), 64));
    Faces.RightCell = static_cast<i32*>(_aligned_malloc(Faces.Size * sizeof(i32), 64));
    Faces.Type = static_cast<FaceType*>(_aligned_malloc(Faces.Size * sizeof(FaceType), 64));
    Faces.Normal = static_cast<vec2*>(_aligned_malloc(Faces.Size * sizeof(vec2), 64));
    Faces.Tangent = static_cast<vec2*>(_aligned_malloc(Faces.Size * sizeof(vec2), 64));

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
    }

    std::fill(Faces.Type, Faces.Type + Faces.Size, FaceType::NoBoundry);

    // Setting Up Default Cell Data
    std::fill(Cells.TopFaceIdx, Cells.TopFaceIdx + Cells.SizeFlat, -1);
    std::fill(Cells.BottomFaceIdx, Cells.BottomFaceIdx + Cells.SizeFlat, -1);
    std::fill(Cells.LeftFaceIdx, Cells.LeftFaceIdx + Cells.SizeFlat, -1);
    std::fill(Cells.RightFaceIdx, Cells.RightFaceIdx + Cells.SizeFlat, -1);

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

    std::fill(Cells.Type, Cells.Type + Cells.SizeFlat, CellType::Fluid);
}

Mesh::Mesh(const Mesh& Other)
{
    this->Cells.Size = Other.Cells.Size;
    this->Cells.SizeFlat = Other.Cells.SizeFlat;
    
    this->Cells.Type = static_cast<CellType*>(_aligned_malloc(Cells.SizeFlat * sizeof(CellType), 64));
    this->Cells.TopFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    this->Cells.BottomFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    this->Cells.LeftFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    this->Cells.RightFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    
    this->Faces.Size = Other.Faces.Size;
    
    this->Faces.LeftCell = static_cast<i32*>(_aligned_malloc(Faces.Size * sizeof(i32), 64));
    this->Faces.RightCell = static_cast<i32*>(_aligned_malloc(Faces.Size * sizeof(i32), 64));
    this->Faces.Type = static_cast<FaceType*>(_aligned_malloc(Faces.Size * sizeof(FaceType), 64));
    this->Faces.Normal = static_cast<vec2*>(_aligned_malloc(Faces.Size * sizeof(vec2), 64));
    this->Faces.Tangent = static_cast<vec2*>(_aligned_malloc(Faces.Size * sizeof(vec2), 64));

    memcpy(this->Cells.Type, Other.Cells.Type, Cells.SizeFlat * sizeof(CellType));
    memcpy(this->Cells.TopFaceIdx, Other.Cells.TopFaceIdx, Cells.SizeFlat * sizeof(i32));
    memcpy(this->Cells.BottomFaceIdx, Other.Cells.BottomFaceIdx, Cells.SizeFlat * sizeof(i32));
    memcpy(this->Cells.LeftFaceIdx, Other.Cells.LeftFaceIdx, Cells.SizeFlat * sizeof(i32));
    memcpy(this->Cells.RightFaceIdx, Other.Cells.RightFaceIdx, Cells.SizeFlat * sizeof(i32));

    memcpy(this->Faces.LeftCell, Other.Faces.LeftCell, Faces.Size * sizeof(i32));
    memcpy(this->Faces.RightCell, Other.Faces.RightCell, Faces.Size * sizeof(i32));
    memcpy(this->Faces.Type, Other.Faces.Type, Faces.Size * sizeof(FaceType));
    memcpy(this->Faces.Normal, Other.Faces.Normal, Faces.Size * sizeof(vec2));
    memcpy(this->Faces.Tangent, Other.Faces.Tangent, Faces.Size * sizeof(vec2));
}

Mesh::Mesh(Mesh&& Other)
{
    // Transferring Ownership To New Object
    this->Cells.Size = Other.Cells.Size;
    this->Cells.SizeFlat = Other.Cells.SizeFlat;

    this->Cells.Type = Other.Cells.Type;
    this->Cells.TopFaceIdx = Other.Cells.TopFaceIdx;
    this->Cells.BottomFaceIdx = Other.Cells.BottomFaceIdx;
    this->Cells.LeftFaceIdx = Other.Cells.LeftFaceIdx;
    this->Cells.RightFaceIdx = Other.Cells.RightFaceIdx;

    this->Faces.Size = Other.Faces.Size;

    this->Faces.LeftCell = Other.Faces.LeftCell;
    this->Faces.RightCell = Other.Faces.RightCell;
    this->Faces.Type = Other.Faces.Type;
    this->Faces.Normal = Other.Faces.Normal;
    this->Faces.Tangent = Other.Faces.Tangent;

    // Setting Other Object To Null State
    Other.Cells.Size = ivec2(0, 0);
    Other.Cells.SizeFlat = 0;

    Other.Cells.Type = nullptr;
    Other.Cells.TopFaceIdx = nullptr;
    Other.Cells.BottomFaceIdx = nullptr;
    Other.Cells.LeftFaceIdx = nullptr;
    Other.Cells.RightFaceIdx = nullptr;

    Other.Faces.Size = 0;

    Other.Faces.LeftCell = nullptr;
    Other.Faces.RightCell = nullptr;
    Other.Faces.Type = nullptr;
    Other.Faces.Normal = nullptr;
    Other.Faces.Tangent = nullptr;
}

Mesh& Mesh::operator=(const Mesh& Other)
{
    if (this == &Other)
        return *this;
    
    // Freeing Owned Resources From This Object
    _aligned_free(this->Cells.Type);
    _aligned_free(this->Cells.TopFaceIdx);
    _aligned_free(this->Cells.BottomFaceIdx);
    _aligned_free(this->Cells.LeftFaceIdx);
    _aligned_free(this->Cells.RightFaceIdx);

    _aligned_free(this->Faces.LeftCell);
    _aligned_free(this->Faces.RightCell);
    _aligned_free(this->Faces.Type);
    _aligned_free(this->Faces.Normal);
    _aligned_free(this->Faces.Tangent);

    // Setting Original Object To Null State
    this->Cells.Size = ivec2(0, 0);
    this->Cells.SizeFlat = 0;
    
    this->Cells.Type = nullptr;
    this->Cells.TopFaceIdx = nullptr;
    this->Cells.BottomFaceIdx = nullptr;
    this->Cells.LeftFaceIdx = nullptr;
    this->Cells.RightFaceIdx = nullptr;
    
    this->Faces.Size = 0;
    
    this->Faces.LeftCell = nullptr;
    this->Faces.RightCell = nullptr;
    this->Faces.Type = nullptr;
    this->Faces.Normal = nullptr;
    this->Faces.Tangent = nullptr;
    
    //Copy Other Across To This Object
    this->Cells.Size = Other.Cells.Size;
    this->Cells.SizeFlat = Other.Cells.SizeFlat;

    this->Cells.Type = static_cast<CellType*>(_aligned_malloc(Cells.SizeFlat * sizeof(CellType), 64));
    this->Cells.TopFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    this->Cells.BottomFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    this->Cells.LeftFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));
    this->Cells.RightFaceIdx = static_cast<i32*>(_aligned_malloc(Cells.SizeFlat * sizeof(i32), 64));

    this->Faces.Size = Other.Faces.Size;

    this->Faces.LeftCell = static_cast<i32*>(_aligned_malloc(Faces.Size * sizeof(i32), 64));
    this->Faces.RightCell = static_cast<i32*>(_aligned_malloc(Faces.Size * sizeof(i32), 64));
    this->Faces.Type = static_cast<FaceType*>(_aligned_malloc(Faces.Size * sizeof(FaceType), 64));
    this->Faces.Normal = static_cast<vec2*>(_aligned_malloc(Faces.Size * sizeof(vec2), 64));
    this->Faces.Tangent = static_cast<vec2*>(_aligned_malloc(Faces.Size * sizeof(vec2), 64));

    memcpy(this->Cells.Type, Other.Cells.Type, Cells.SizeFlat * sizeof(CellType));
    memcpy(this->Cells.TopFaceIdx, Other.Cells.TopFaceIdx, Cells.SizeFlat * sizeof(i32));
    memcpy(this->Cells.BottomFaceIdx, Other.Cells.BottomFaceIdx, Cells.SizeFlat * sizeof(i32));
    memcpy(this->Cells.LeftFaceIdx, Other.Cells.LeftFaceIdx, Cells.SizeFlat * sizeof(i32));
    memcpy(this->Cells.RightFaceIdx, Other.Cells.RightFaceIdx, Cells.SizeFlat * sizeof(i32));

    memcpy(this->Faces.LeftCell, Other.Faces.LeftCell, Faces.Size * sizeof(i32));
    memcpy(this->Faces.RightCell, Other.Faces.RightCell, Faces.Size * sizeof(i32));
    memcpy(this->Faces.Type, Other.Faces.Type, Faces.Size * sizeof(FaceType));
    memcpy(this->Faces.Normal, Other.Faces.Normal, Faces.Size * sizeof(vec2));
    memcpy(this->Faces.Tangent, Other.Faces.Tangent, Faces.Size * sizeof(vec2));

    return *this;
}

Mesh& Mesh::operator=(Mesh&& Other)
{
    if (this == &Other)
        return *this;
    
    // Freeing Owned Resources From This Object
    _aligned_free(this->Cells.Type);
    _aligned_free(this->Cells.TopFaceIdx);
    _aligned_free(this->Cells.BottomFaceIdx);
    _aligned_free(this->Cells.LeftFaceIdx);
    _aligned_free(this->Cells.RightFaceIdx);

    _aligned_free(this->Faces.LeftCell);
    _aligned_free(this->Faces.RightCell);
    _aligned_free(this->Faces.Type);
    _aligned_free(this->Faces.Normal);
    _aligned_free(this->Faces.Tangent);

    // Setting Original Object To Null State
    this->Cells.Size = ivec2(0, 0);
    this->Cells.SizeFlat = 0;

    this->Cells.Type = nullptr;
    this->Cells.TopFaceIdx = nullptr;
    this->Cells.BottomFaceIdx = nullptr;
    this->Cells.LeftFaceIdx = nullptr;
    this->Cells.RightFaceIdx = nullptr;

    this->Faces.Size = 0;

    this->Faces.LeftCell = nullptr;
    this->Faces.RightCell = nullptr;
    this->Faces.Type = nullptr;
    this->Faces.Normal = nullptr;
    this->Faces.Tangent = nullptr;

    // Transfer Ownership Of Resources From Other
    this->Cells.Size = Other.Cells.Size;
    this->Cells.SizeFlat = Other.Cells.SizeFlat;
    
    this->Cells.Type = Other.Cells.Type;
    this->Cells.TopFaceIdx = Other.Cells.TopFaceIdx;
    this->Cells.BottomFaceIdx = Other.Cells.BottomFaceIdx;
    this->Cells.LeftFaceIdx = Other.Cells.LeftFaceIdx;
    this->Cells.RightFaceIdx = Other.Cells.RightFaceIdx;
    
    this->Faces.Size = Other.Faces.Size;
    
    this->Faces.LeftCell = Other.Faces.LeftCell;
    this->Faces.RightCell = Other.Faces.RightCell;
    this->Faces.Type = Other.Faces.Type;
    this->Faces.Normal = Other.Faces.Normal;

    this->Faces.Tangent = Other.Faces.Tangent;

    // Setting Other Object To Null State
    Other.Cells.Size = ivec2(0, 0);
    Other.Cells.SizeFlat = 0;

    Other.Cells.Type = nullptr;
    Other.Cells.TopFaceIdx = nullptr;
    Other.Cells.BottomFaceIdx = nullptr;
    Other.Cells.LeftFaceIdx = nullptr;
    Other.Cells.RightFaceIdx = nullptr;

    Other.Faces.Size = 0;

    Other.Faces.LeftCell = nullptr;
    Other.Faces.RightCell = nullptr;
    Other.Faces.Type = nullptr;
    Other.Faces.Normal = nullptr;
    Other.Faces.Tangent = nullptr;

    return *this;
}

Mesh::~Mesh()
{
    _aligned_free(Cells.Type);
    _aligned_free(Cells.TopFaceIdx);
    _aligned_free(Cells.BottomFaceIdx);
    _aligned_free(Cells.LeftFaceIdx);
    _aligned_free(Cells.RightFaceIdx);

    _aligned_free(Faces.LeftCell);
    _aligned_free(Faces.RightCell);
    _aligned_free(Faces.Type);
    _aligned_free(Faces.Normal);
    _aligned_free(Faces.Tangent);
}

i32 Mesh::GetLeftCell(i32 FaceIndex) const
{
    return Faces.LeftCell[FaceIndex];
}

i32 Mesh::GetRightCell(i32 FaceIndex) const
{
    return Faces.RightCell[FaceIndex];
}

Mesh::FaceType Mesh::GetFaceType(i32 FaceIndex) const
{
    return Faces.Type[FaceIndex];
}

vec2 Mesh::GetNormal(i32 FaceIndex) const
{
    return Faces.Normal[FaceIndex];
}

vec2 Mesh::GetTangent(i32 FaceIndex) const
{
    return Faces.Tangent[FaceIndex];
}

Mesh::CellType Mesh::GetCellType(i32 CellIndex) const
{
    return Cells.Type[CellIndex];
}

i32 Mesh::GetFacesSize() const
{
    return Faces.Size;
}

ivec2 Mesh::GetGridSize() const
{
    return Cells.Size;
}

i32 Mesh::GetGridSizeFlat() const
{
    return Cells.SizeFlat;
}

i32 Mesh::GetCellIndex(const ivec2& Position) const
{
    return Position.y * Cells.Size.x + Position.x;
}

ivec2 Mesh::GetCellPosition(i32 Index) const
{
    return ivec2(Index / Cells.Size.x, Index % Cells.Size.x);
}

bool Mesh::IsValidCell(i32 Index) const
{
    return Index >= 0 && Index < Cells.SizeFlat;
}

bool Mesh::IsValidCell(const ivec2& Position) const
{
    return Position.x >= 0 && Position.y >= 0 && Position.x < Cells.Size.x && Position.y < Cells.Size.y;
}

void Mesh::SetFaceType(i32 Index, FaceType Type)
{

}

void Mesh::SetCellType(i32 Index, CellType Type, FaceType BorderType)
{

}

void Mesh::SetCellType(const ivec2& Position, CellType Type, FaceType BorderType)
{

}

void Mesh::AddRect(const ivec2& BottomLeft, const ivec2& Size, CellType Type, FaceType BorderType)
{
    for (i32 x = BottomLeft.x; x < BottomLeft.x + Size.x; ++x)
    {
        i32 y1 = BottomLeft.y;
        i32 y2 = BottomLeft.y + Size.y - 1;

        SetCellType(ivec2(x, y1), Type, BorderType);
        SetCellType(ivec2(x, y2), Type, BorderType);
    }

    for (i32 y = BottomLeft.y; y < BottomLeft.y + Size.y; ++y)
    {
        i32 x1 = BottomLeft.x;
        i32 x2 = BottomLeft.x + Size.x - 1;

        SetCellType(ivec2(x1, y), Type, BorderType);
        SetCellType(ivec2(x2, y), Type, BorderType);
    }
}

void Mesh::AddRectFilled(const ivec2& BottomLeft, const ivec2& Size, CellType Type, FaceType BorderType)
{
    for (i32 x = BottomLeft.x; x < BottomLeft.x + Size.x; ++x)
    {
        for (i32 y = BottomLeft.y; y < BottomLeft.y + Size.y; ++y)
        {
            SetCellType(ivec2(x, y), Type, BorderType);
        }
    }
}

void Mesh::AddCircle(const vec2& Center, f32 r, CellType Type, FaceType BorderType)
{
    for (i32 x = 0; x < Cells.Size.x; ++x)
    {
        for (i32 y = 0; y < Cells.Size.y; ++y)
        {
            vec2 Position = vec2(x, y) + vec2(0.5f, 0.5f);

            if (glm::distance(Center, Position) > r)
                continue;

            SetCellType(ivec2(x, y), Type, BorderType);
        }
    }
}