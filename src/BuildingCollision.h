#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

// Keeps a walker (a circle on the ground plane) outside building footprints.
// Footprints come from the town's .collision file (format in the README).
class BuildingCollision
{
public:
    // Returns false (and keeps no footprints) when the file cannot be read
    bool Load(const std::string &path);

    // Moves `position` (x, z) out of any footprint it overlaps, sliding along walls
    glm::vec2 Resolve(glm::vec2 position, float radius) const;

    size_t GetFootprintCount() const { return m_Footprints.size(); }

private:
    struct Footprint
    {
        std::vector<glm::vec2> points; // counter-clockwise, not closed
        glm::vec2 min, max;            // bounding box for a cheap reject
    };

    static bool Contains(const Footprint &footprint, glm::vec2 point);
    static glm::vec2 ClosestPointOnOutline(const Footprint &footprint, glm::vec2 point);

    std::vector<Footprint> m_Footprints;
};
