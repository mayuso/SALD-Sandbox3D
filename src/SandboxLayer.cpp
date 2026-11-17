#include "SandboxLayer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace {
// Matches the old speed (0.1 degrees per scene draw, 5 draws per frame, 60 Hz)
constexpr GLfloat kBlackhawkDegreesPerSecond = 30.0f;
// World Y of the demo floor mesh (drawn translated by -2 on Y)
constexpr GLfloat kFloorHeight = -2.0f;
} // namespace

SandboxLayer::SandboxLayer()
    : Layer("SandboxLayer"),
      m_Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
               -90.0f, 0.0f, 4.0f, 10.0f),
      shinyMaterial(1.0f, 32.0f), dullMaterial(0.3f, 4.0f) {
  CreateObjects();
  CreateShaders();

  // Walk on the floor by default: Tab toggles fly mode, Escape frees the cursor
  m_Camera.SetGroundHeightFunction(
      [](float, float) { return kFloorHeight; });
  m_Camera.SetRunMultiplier(3.0f);
  m_Camera.SetMode(Sald::Camera::Mode::Walk);
  SetCursorCaptured(true);

  Sald::TextureManager::NewTexture("brick", "Textures/brick.png");
  Sald::TextureManager::NewTexture("dirt", "Textures/dirt.png");

  blackhawk.Load("Models/uh60.obj");
  xwing.Load("Models/x-wing.obj");

  directionalLight = Sald::DirectionalLight(2048, 2048, 1.0f, 0.53f, 0.3f, 0.1f,
                                            0.9f, -10.0f, -12.0f, 18.5f);

  pointLights[0] =
      Sald::PointLight(1024, 1024, 0.01f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                       1.0f, 2.0f, 0.0f, 0.3f, 0.2f, 0.1f, 0);
  pointLightCount++;
  pointLights[1] =
      Sald::PointLight(1024, 1024, 0.01f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                       -4.0f, 3.0f, 0.0f, 0.3f, 0.2f, 0.1f, 1);
  pointLightCount++;

  spotLights[0] = Sald::SpotLight(1024, 1024, 0.01f, 100.0f, 1.0f, 1.0f, 1.0f,
                                  0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                  0.0f, 1.0f, 0.0f, 0.0f, 20.0f, 0);
  spotLightCount++;

  skybox = Sald::Skybox({"Textures/Skybox/cupertin-lake_rt.tga",
                         "Textures/Skybox/cupertin-lake_lf.tga",
                         "Textures/Skybox/cupertin-lake_up.tga",
                         "Textures/Skybox/cupertin-lake_dn.tga",
                         "Textures/Skybox/cupertin-lake_bk.tga",
                         "Textures/Skybox/cupertin-lake_ft.tga"});
}

SandboxLayer::~SandboxLayer() {
  for (Sald::Mesh *mesh : meshList)
    delete mesh;
}

void SandboxLayer::CreateObjects() {
  unsigned int indices[] = {0, 3, 1, 1, 3, 2, 2, 3, 0, 0, 1, 2};

  GLfloat vertices[] = {
      //  x      y      z     u     v     nx    ny    nz
      -1.0f, -1.0f, -0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
       0.0f, -1.0f,  1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
       1.0f, -1.0f, -0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
       0.0f,  1.0f,  0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f};

  unsigned int floorIndices[] = {0, 2, 1, 1, 2, 3};

  GLfloat floorVertices[] = {
      //   x      y      z      u      v     nx     ny    nz
      -10.0f, 0.0f, -10.0f,  0.0f,  0.0f, 0.0f,  1.0f, 0.0f,
       10.0f, 0.0f, -10.0f, 10.0f,  0.0f, 0.0f,  1.0f, 0.0f,
      -10.0f, 0.0f,  10.0f,  0.0f, 10.0f, 0.0f,  1.0f, 0.0f,
       10.0f, 0.0f,  10.0f, 10.0f, 10.0f, 0.0f,  1.0f, 0.0f};

  CalcAverageNormals(indices, 12, vertices, 32, 8, 5);

  // [0] pyramid (drawn twice with different transforms), [1] floor
  meshList.push_back(new Sald::Mesh(vertices, indices, 32, 12, true));
  meshList.push_back(new Sald::Mesh(floorVertices, floorIndices, 32, 6, true));
}

void SandboxLayer::CalcAverageNormals(unsigned int *indices,
                                      unsigned int indiceCount,
                                      GLfloat *vertices,
                                      unsigned int verticeCount,
                                      unsigned int vLength,
                                      unsigned int normalOffset) {
  for (size_t i = 0; i < indiceCount; i += 3) {
    unsigned int in0 = indices[i] * vLength;
    unsigned int in1 = indices[i + 1] * vLength;
    unsigned int in2 = indices[i + 2] * vLength;
    glm::vec3 v1(vertices[in1] - vertices[in0],
                 vertices[in1 + 1] - vertices[in0 + 1],
                 vertices[in1 + 2] - vertices[in0 + 2]);
    glm::vec3 v2(vertices[in2] - vertices[in0],
                 vertices[in2 + 1] - vertices[in0 + 1],
                 vertices[in2 + 2] - vertices[in0 + 2]);
    // Outward normal for this mesh's winding
    glm::vec3 normal = glm::cross(v2, v1);
    normal = glm::normalize(normal);

    in0 += normalOffset;
    in1 += normalOffset;
    in2 += normalOffset;

    vertices[in0] += normal.x;
    vertices[in0 + 1] += normal.y;
    vertices[in0 + 2] += normal.z;

    vertices[in1] += normal.x;
    vertices[in1 + 1] += normal.y;
    vertices[in1 + 2] += normal.z;

    vertices[in2] += normal.x;
    vertices[in2 + 1] += normal.y;
    vertices[in2 + 2] += normal.z;
  }
  for (size_t i = 0; i < verticeCount / vLength; i++) {
    unsigned int nOffset = i * vLength + normalOffset;
    glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1],
                  vertices[nOffset + 2]);
    vec = glm::normalize(vec);
    vertices[nOffset] = vec.x;
    vertices[nOffset + 1] = vec.y;
    vertices[nOffset + 2] = vec.z;
  }
}

void SandboxLayer::CreateShaders() {
  Sald::ShaderManager::NewShader("shader", "Shaders/shader.vert",
                                 "Shaders/shader.frag");
  Sald::ShaderManager::NewShader("light", "Shaders/directional_shadow_map.vert",
                                 "Shaders/directional_shadow_map.frag");
  Sald::ShaderManager::NewShader("shadowMap",
                                 "Shaders/omni_directional_shadow_map.vert",
                                 "Shaders/omni_directional_shadow_map.geom",
                                 "Shaders/omni_directional_shadow_map.frag");
}

void SandboxLayer::OnUpdate(Sald::Timestep timestep) {
  m_Camera.OnUpdate(timestep);

  blackhawkAngle = std::fmod(
      blackhawkAngle + kBlackhawkDegreesPerSecond * (float)timestep, 360.0f);

  // Keep the spot light attached to the camera before any pass uses it
  glm::vec3 lowerLight = m_Camera.GetCameraPosition();
  lowerLight.y -= 0.2f;
  spotLights[0].Move(lowerLight, m_Camera.GetCameraDirection());

  LightingPass();
  RenderPass(m_Camera.CalculateViewMatrix());

  m_ShadersValidated = true;
}

bool SandboxLayer::OnKeyPressed(Sald::KeyPressedEvent &event) {
  // Event based (not polled each frame) so holding the key toggles only once
  if (event.IsRepeat())
    return false;

  switch (event.GetKeyCode()) {
  case Sald::Key::L:
    spotLights[0].Toggle();
    return true;
  case Sald::Key::Tab:
    m_Camera.SetMode(m_Camera.GetMode() == Sald::Camera::Mode::Walk
                         ? Sald::Camera::Mode::Fly
                         : Sald::Camera::Mode::Walk);
    return true;
  case Sald::Key::Escape:
    SetCursorCaptured(!m_CursorCaptured);
    return true;
  default:
    return false;
  }
}

void SandboxLayer::SetCursorCaptured(bool captured) {
  m_CursorCaptured = captured;
  Sald::Application::GetInstance().GetWindow()->SetCursorEnabled(!captured);
  m_Camera.SetMouseLookEnabled(captured);
}

void SandboxLayer::OnEvent(Sald::Event &event) {
  Sald::EventDispatcher dispatcher(event);
  dispatcher.Dispatch<Sald::KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));
}

void SandboxLayer::LightingPass() {
  DirectionalShadowMapPass(&directionalLight);
  for (size_t i = 0; i < pointLightCount; i++) {
    OmniShadowMapPass(&pointLights[i]);
  }
  for (size_t i = 0; i < spotLightCount; i++) {
    OmniShadowMapPass(&spotLights[i]);
  }
}

void SandboxLayer::DirectionalShadowMapPass(Sald::DirectionalLight *light) {
  Sald::Shader *directionalShadowShader =
      Sald::ShaderManager::GetShader("light");
  directionalShadowShader->Bind();

  light->GetShadowMap()->CreateFrameBuffer();

  glm::mat4 calculatedLightTransform = light->CalculateLightTransform();
  directionalShadowShader->SetDirectionalLightTransform(
      &calculatedLightTransform);

  if (!m_ShadersValidated)
    directionalShadowShader->Validate();
  RenderScene(directionalShadowShader, false);
  light->GetShadowMap()->DeleteFrameBuffer();
}

void SandboxLayer::OmniShadowMapPass(Sald::PointLight *light) {
  Sald::Shader *omniShadowShader = Sald::ShaderManager::GetShader("shadowMap");
  omniShadowShader->Bind();
  light->GetShadowMap()->CreateFrameBuffer();

  glm::vec3 lightPosition = light->GetPosition();
  omniShadowShader->SetFloat3("lightPos", lightPosition.x, lightPosition.y,
                              lightPosition.z);
  omniShadowShader->SetFloat("farPlane", light->GetFarPlane());

  omniShadowShader->SetOmniLightMatrices(light->CalculateLightTransform());
  if (!m_ShadersValidated)
    omniShadowShader->Validate();
  RenderScene(omniShadowShader, false);

  light->GetShadowMap()->DeleteFrameBuffer();
}

void SandboxLayer::RenderPass(glm::mat4 viewMatrix) {
  // The shadow passes leave the viewport at the shadow map size, so restore it
  // to the current framebuffer size (which changes on resize / HiDPI)
  auto window = Sald::Application::GetInstance().GetWindow();
  int width = (int)window->GetBufferWidth();
  int height = (int)window->GetBufferHeight();
  if (width <= 0 || height <= 0)
    return;
  if (width != m_ViewportWidth || height != m_ViewportHeight) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_Projection = glm::perspective(glm::radians(60.0f),
                                    (float)width / (float)height, 0.1f, 100.0f);
  }

  Sald::RenderCommand::SetViewport(0, 0, width, height);
  Sald::RenderCommand::SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  Sald::RenderCommand::Clear();

  skybox.DrawSkybox(viewMatrix, m_Projection);

  Sald::Shader *shader = Sald::ShaderManager::GetShader("shader");
  shader->Bind();

  shader->SetMat4("projection", m_Projection);
  shader->SetMat4("view", viewMatrix);
  glm::vec3 eyePosition = m_Camera.GetCameraPosition();
  shader->SetFloat3("eyePosition", eyePosition.x, eyePosition.y,
                    eyePosition.z);

  // Texture units: 0 skybox, 1 object texture, 2 directional shadow map,
  // 3.. omni shadow maps (point lights first, then spot lights)
  directionalLight.UseLight(shader);
  for (unsigned int i = 0; i < pointLightCount; i++)
    pointLights[i].UseLight(shader, pointLightCount, 3, 0);
  for (unsigned int i = 0; i < spotLightCount; i++)
    spotLights[i].UseLight(shader, spotLightCount, 3 + pointLightCount,
                           pointLightCount);

  glm::mat4 calculatedLightTransform =
      directionalLight.CalculateLightTransform();
  shader->SetDirectionalLightTransform(&calculatedLightTransform);

  directionalLight.GetShadowMap()->Read(GL_TEXTURE2);

  shader->SetTexture(1);
  shader->SetDirectionalShadowMap(2);

  if (!m_ShadersValidated)
    shader->Validate();
  RenderScene(shader, true);
}

void SandboxLayer::SetModelMatrix(Sald::Shader *shader, const glm::mat4 &model,
                                  bool lightingPass) {
  shader->SetMat4("model", model);
  // Computed once per object here instead of per vertex in shader.vert
  if (lightingPass)
    shader->SetMat4("normalMatrix",
                    glm::mat4(glm::transpose(glm::inverse(glm::mat3(model)))));
}

void SandboxLayer::RenderScene(Sald::Shader *currentShader,
                               bool lightingPass) {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
  SetModelMatrix(currentShader, model, lightingPass);
  Sald::TextureManager::GetTexture("brick")->Use(1);
  if (lightingPass)
    shinyMaterial.UseMaterial(currentShader);
  meshList[0]->RenderMesh();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-4.0f, 1.0f, -4.0f));
  SetModelMatrix(currentShader, model, lightingPass);
  Sald::TextureManager::GetTexture("dirt")->Use(1);
  if (lightingPass)
    dullMaterial.UseMaterial(currentShader);
  meshList[0]->RenderMesh();

  model = glm::mat4(1.0f);
  model = glm::rotate(model, glm::radians(-blackhawkAngle),
                      glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::translate(model, glm::vec3(-8.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
  SetModelMatrix(currentShader, model, lightingPass);
  if (lightingPass)
    shinyMaterial.UseMaterial(currentShader);
  blackhawk.Render();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-7.0f, 1.0f, 10.0f));
  model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
  SetModelMatrix(currentShader, model, lightingPass);
  if (lightingPass)
    shinyMaterial.UseMaterial(currentShader);
  xwing.Render();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
  SetModelMatrix(currentShader, model, lightingPass);
  Sald::TextureManager::GetTexture("dirt")->Use(1);
  if (lightingPass)
    shinyMaterial.UseMaterial(currentShader);
  meshList[1]->RenderMesh();
}
