#pragma once
#include "sald.h"

#include <glm/glm.hpp>

#include <vector>

class SandboxLayer : public Sald::Layer
{
public:
    SandboxLayer();
    ~SandboxLayer();

    void OnUpdate(Sald::Timestep timestep) override;
    void OnEvent(Sald::Event &event) override;

private:
    void CalcAverageNormals(unsigned int *indices, unsigned int indiceCount, GLfloat *vertices, unsigned int verticeCount,
                            unsigned int vLength, unsigned int normalOffset);
    void CreateObjects();
    void CreateShaders();
    // lightingPass: the main shader; depth-only shadow shaders have no
    // material or normalMatrix uniforms
    void RenderScene(Sald::Shader *currentShader, bool lightingPass);
    void SetModelMatrix(Sald::Shader *shader, const glm::mat4 &model, bool lightingPass);
    void DirectionalShadowMapPass(Sald::DirectionalLight *light);
    void OmniShadowMapPass(Sald::PointLight *light);
    void RenderPass(glm::mat4 viewMatrix);
    void LightingPass();
    bool OnKeyPressed(Sald::KeyPressedEvent &event);
    // Captured: cursor hidden and mouse turns the camera
    void SetCursorCaptured(bool captured);

    Sald::Camera m_Camera;

    std::vector<Sald::Mesh *> meshList;

    Sald::Material shinyMaterial;
    Sald::Material dullMaterial;

    Sald::Model blackhawk, xwing;

    Sald::DirectionalLight directionalLight;
    Sald::PointLight pointLights[MAX_POINT_LIGHTS];
    Sald::SpotLight spotLights[MAX_SPOT_LIGHTS];

    Sald::Skybox skybox;

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount = 0;

    // Degrees; advanced once per frame in OnUpdate so every pass draws the same pose
    GLfloat blackhawkAngle = 0.0f;

    // glValidateProgram is a debug check; run it on the first frame only
    bool m_ShadersValidated = false;

    bool m_CursorCaptured = false;

    glm::mat4 m_Projection = glm::mat4(1.0f);
    int m_ViewportWidth = 0, m_ViewportHeight = 0;
};
