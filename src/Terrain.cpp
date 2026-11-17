#include "Terrain.h"

#include <algorithm>
#include <fstream>

bool Terrain::Load(const std::string &path)
{
    m_Heights.clear();
    std::ifstream file(path);
    int cellsX = 0, cellsZ = 0;
    float minX = 0.0f, minZ = 0.0f, cellSizeX = 0.0f, cellSizeZ = 0.0f;
    if (!(file >> cellsX >> cellsZ >> minX >> minZ >> cellSizeX >> cellSizeZ) ||
        cellsX < 1 || cellsZ < 1 || cellSizeX <= 0.0f || cellSizeZ <= 0.0f)
        return false;

    std::vector<float> heights((size_t)(cellsX + 1) * (cellsZ + 1));
    for (float &height : heights)
        if (!(file >> height))
            return false;

    m_CellsX = cellsX;
    m_CellsZ = cellsZ;
    m_MinX = minX;
    m_MinZ = minZ;
    m_CellSizeX = cellSizeX;
    m_CellSizeZ = cellSizeZ;
    m_Heights = std::move(heights);
    return true;
}

float Terrain::GetHeight(float x, float z) const
{
    if (m_Heights.empty())
        return 0.0f;

    float fx = std::clamp((x - m_MinX) / m_CellSizeX, 0.0f, m_CellsX - 1e-4f);
    float fz = std::clamp((z - m_MinZ) / m_CellSizeZ, 0.0f, m_CellsZ - 1e-4f);
    int i = (int)fx, j = (int)fz;
    float u = fx - i, v = fz - j;

    // Each cell is two triangles split along its (i, j)-(i + 1, j + 1) diagonal,
    // the same split as the ground mesh
    float h00 = At(i, j), h10 = At(i + 1, j), h01 = At(i, j + 1), h11 = At(i + 1, j + 1);
    if (u >= v)
        return h00 + u * (h10 - h00) + v * (h11 - h10);
    return h00 + v * (h01 - h00) + u * (h11 - h01);
}
