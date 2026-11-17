#pragma once
#include "sald.h"

#include "BuildingCollision.h"
#include "Terrain.h"

#include <glm/glm.hpp>

#include <string>

// Walk around the town in Models/<name>.* (file formats in the README)
class TownLayer : public Sald::Layer
{
public:
    // Loads Models/<name>.obj, .spawn, .collision and .terrain
    explicit TownLayer(const std::string &name);

    void OnUpdate(Sald::Timestep timestep) override;
    void OnEvent(Sald::Event &event) override;

private:
    bool OnKeyPressed(Sald::KeyPressedEvent &event);
    void SetCursorCaptured(bool captured);
    void LoadSpawn(const std::string &path);
    glm::mat4 CalculateShadowTransform();
    void ShadowPass(const glm::mat4 &lightTransform);
    void RenderPass(const glm::mat4 &lightTransform);
    void UpdateProjection();

    Sald::Camera m_Camera;
    Sald::Model m_Town;
    BuildingCollision m_Buildings;
    Terrain m_Terrain;
    Sald::Material m_Material;
    Sald::DirectionalLight m_Sun;
    glm::vec3 m_SunDirection;

    // Walkable area: half extents of the generated ground plane around the origin
    glm::vec2 m_HalfExtents = {0.0f, 0.0f};
    glm::vec3 m_Spawn = {0.0f, 0.0f, 0.0f};

    bool m_CursorCaptured = false;
    bool m_ShadersValidated = false;

    glm::mat4 m_Projection = glm::mat4(1.0f);
    int m_ViewportWidth = 0, m_ViewportHeight = 0;
};
