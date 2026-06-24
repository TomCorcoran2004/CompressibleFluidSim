#pragma once
#include <glm/glm.hpp>
#include <vector>

class Mesh
{
public:
    enum class FaceType : u8
    {
        Interior = 0b00000001,
        Wall =     0b00000010,
        Outflow =  0b00000100,
        Inflow =   0b00001000,
        Periodic = 0b00010000,
    };

    enum class CellType : u8
    {
        Solid = 0,
        Fluid = 1,
    };

    struct MeshConfig
    {
        ivec2 MeshDimensions = ivec2(0, 0);
    };

    // Constructors & Destructors
    Mesh();
    Mesh(const MeshConfig& Config);
    Mesh(const Mesh& Other);
    Mesh(Mesh&& Other);
    Mesh& operator=(const Mesh& Other);
    Mesh& operator=(Mesh&& Other);
    ~Mesh();

    //Face Index Helpers
    bool IsValidFace(i32 FaceIndex);

    // Face Getters
    i32 GetLeftCell(i32 FaceIndex) const;
    i32 GetRightCell(i32 FaceIndex) const;
    FaceType GetFaceType(i32 FaceIndex) const;
    vec2 GetNormal(i32 FaceIndex) const;
    vec2 GetTangent(i32 FaceIndex) const;

    // Face Raw Data Getters
    const i32* GetLeftCells();
    const i32* GetRightCells();
    const FaceType* GetFaceType();
    const vec2* GetNormals();
    const vec2* GetTangents();
    i32 GetFacesSize() const;
    
    // Cell Getters
    CellType GetCellType(i32 CellIndex) const;

    // Cell Raw Data Getters
    CellType* GetCellTypes();
    i32* GetTopFaceIdxs();
    i32* GetBottomFaceIdxs();
    i32* GetLeftFaceIdxs();
    i32* GetRightFaceIdxs();
    ivec2 GetGridSize() const;
    i32 GetGridSizeFlat() const;

    // Cell Index Helpers
    i32 GetCellIndex(const ivec2& Position) const;
    ivec2 GetCellPosition(i32 Index) const;

    bool IsValidCell(i32 Index) const;
    bool IsValidCell(const ivec2& Position) const;

    // Mesh Editors
    void SetVerticallyPeriodic();
    void SetHorizontallyPeriodic();
    
    void SetFaceType(i32 Index, FaceType Type);
    void SetCellType(i32 Index, CellType Type, FaceType BorderType);
    void SetCellType(const ivec2& Position, CellType Type, FaceType BorderType);
    void AddRect(const ivec2& BottomLeft, const ivec2& Size, CellType Type, FaceType BorderType);
    void AddRectFilled(const ivec2& BottomLeft, const ivec2& Size, CellType Type, FaceType BorderType);
    void AddCircle(const vec2& Center, f32 r, CellType Type, FaceType BorderType);
    
private:
    struct FacesArray
    {
        i32* LeftCell = nullptr;
        i32* RightCell = nullptr;
        FaceType* Type = nullptr;
        vec2* Normal = nullptr;
        vec2* Tangent = nullptr;

        i32 Size = 0;
    };

    FacesArray Faces = {  };
    
    struct CellsArray
    {
        CellType* Type = nullptr;
        i32* TopFaceIdx = nullptr;
        i32* BottomFaceIdx = nullptr;
        i32* LeftFaceIdx = nullptr;
        i32* RightFaceIdx = nullptr;

        ivec2 Size = ivec2(0, 0);
        i32 SizeFlat = 0;
    };

    CellsArray Cells = {  };
};