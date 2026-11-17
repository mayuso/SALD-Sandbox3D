#pragma once

#include <string>
#include <vector>

// Ground heights from the town's .terrain file (format in the README).
// Interpolates on the same triangles as the town's ground mesh, so a
// walker's feet stay on the visible ground.
class Terrain
{
public:
    // Returns false (and leaves the ground flat at 0) when the file cannot be read
    bool Load(const std::string &path);

    // World Y of the ground at (x, z); clamped to the grid edge outside it
    float GetHeight(float x, float z) const;

private:
    int m_CellsX = 0, m_CellsZ = 0;
    float m_MinX = 0.0f, m_MinZ = 0.0f;
    float m_CellSizeX = 1.0f, m_CellSizeZ = 1.0f;
    // (m_CellsZ + 1) rows of (m_CellsX + 1) heights, rows going +Z
    std::vector<float> m_Heights;

    float At(int i, int j) const { return m_Heights[(size_t)j * (m_CellsX + 1) + i]; }
};
