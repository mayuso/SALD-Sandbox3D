#include "BuildingCollision.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>

bool BuildingCollision::Load(const std::string &path)
{
    m_Footprints.clear();
    std::ifstream file(path);
    if (!file)
        return false;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        size_t count = 0;
        if (!(stream >> count) || count < 3)
            continue;

        Footprint footprint;
        footprint.points.reserve(count);
        glm::vec2 point;
        while (footprint.points.size() < count && stream >> point.x >> point.y)
            footprint.points.push_back(point);
        if (footprint.points.size() != count)
            continue;

        footprint.min = footprint.max = footprint.points[0];
        for (const glm::vec2 &p : footprint.points)
        {
            footprint.min = glm::min(footprint.min, p);
            footprint.max = glm::max(footprint.max, p);
        }
        m_Footprints.push_back(std::move(footprint));
    }
    return true;
}

glm::vec2 BuildingCollision::Resolve(glm::vec2 position, float radius) const
{
    // A push out of one building can push into a neighbour, so repeat a few times
    for (int iteration = 0; iteration < 4; iteration++)
    {
        bool moved = false;
        for (const Footprint &footprint : m_Footprints)
        {
            if (position.x < footprint.min.x - radius || position.x > footprint.max.x + radius ||
                position.y < footprint.min.y - radius || position.y > footprint.max.y + radius)
                continue;

            glm::vec2 closest = ClosestPointOnOutline(footprint, position);
            glm::vec2 offset = position - closest;
            float distance = glm::length(offset);

            if (Contains(footprint, position))
            {
                // Inside: leave through the nearest wall
                glm::vec2 outward = distance > 1e-6f ? -offset / distance : glm::vec2(1.0f, 0.0f);
                position = closest + outward * radius;
                moved = true;
            }
            else if (distance < radius)
            {
                // Touching a wall: push away from it, keeping the movement along it
                glm::vec2 outward = distance > 1e-6f ? offset / distance : glm::vec2(1.0f, 0.0f);
                position = closest + outward * radius;
                moved = true;
            }
        }
        if (!moved)
            break;
    }
    return position;
}

bool BuildingCollision::Contains(const Footprint &footprint, glm::vec2 point)
{
    // Even-odd ray casting along +X
    bool inside = false;
    const std::vector<glm::vec2> &pts = footprint.points;
    for (size_t i = 0, j = pts.size() - 1; i < pts.size(); j = i++)
    {
        if ((pts[i].y > point.y) != (pts[j].y > point.y))
        {
            float crossingX = pts[j].x + (point.y - pts[j].y) * (pts[i].x - pts[j].x) / (pts[i].y - pts[j].y);
            if (point.x < crossingX)
                inside = !inside;
        }
    }
    return inside;
}

glm::vec2 BuildingCollision::ClosestPointOnOutline(const Footprint &footprint, glm::vec2 point)
{
    glm::vec2 best = footprint.points[0];
    float bestDistance2 = std::numeric_limits<float>::max();
    const std::vector<glm::vec2> &pts = footprint.points;
    for (size_t i = 0, j = pts.size() - 1; i < pts.size(); j = i++)
    {
        glm::vec2 a = pts[j], b = pts[i];
        glm::vec2 ab = b - a;
        float length2 = glm::dot(ab, ab);
        float t = length2 > 0.0f ? std::clamp(glm::dot(point - a, ab) / length2, 0.0f, 1.0f) : 0.0f;
        glm::vec2 candidate = a + ab * t;
        glm::vec2 d = point - candidate;
        float distance2 = glm::dot(d, d);
        if (distance2 < bestDistance2)
        {
            bestDistance2 = distance2;
            best = candidate;
        }
    }
    return best;
}
