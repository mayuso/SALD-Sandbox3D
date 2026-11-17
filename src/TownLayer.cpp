#include "TownLayer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>

namespace {
constexpr float kNearPlane = 0.2f;
constexpr float kFarPlane = 1500.0f;
constexpr float kWalkSpeed = 4.0f;    // m/s; Left Shift multiplies it
constexpr float kRunMultiplier = 3.0f;
constexpr float kEyeHeight = 1.7f;
// How close the walker's eye can get to a wall
constexpr float kWalkerRadius = 0.35f;

// Sun shadows cover a square of 2 * kShadowRange metres around the player
constexpr GLuint kShadowMapSize = 4096;
constexpr float kShadowRange = 80.0f;

const glm::vec3 kSkyColor(0.70f, 0.78f, 0.86f);
// visibility = exp(-(distance * density)^2): about 5% left at 600 m
constexpr float kFogDensity = 0.0029f;
} // namespace

TownLayer::TownLayer(const std::string &name)
    : Layer("TownLayer"),
      m_Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f,
               kWalkSpeed, 10.0f),
      m_Material(0.15f, 8.0f),
      m_SunDirection(glm::normalize(glm::vec3(0.45f, -1.0f, 0.35f))) {
  Sald::ShaderManager::NewShader("town", "Shaders/shader.vert",
                                 "Shaders/shader.frag");
  Sald::ShaderManager::NewShader("town_shadow",
                                 "Shaders/directional_shadow_map.vert",
                                 "Shaders/directional_shadow_map.frag");

  m_Town.Load("Models/" + name + ".obj");
  LoadSpawn("Models/" + name + ".spawn");
  if (!m_Buildings.Load("Models/" + name + ".collision"))
    SALD_WARN("No collision file for {0}; buildings can be walked through",
              name);
  if (!m_Terrain.Load("Models/" + name + ".terrain"))
    SALD_WARN("No terrain file for {0}; walking on flat ground", name);

  m_Sun = Sald::DirectionalLight(kShadowMapSize, kShadowMapSize, 1.0f, 0.97f,
                                 0.9f, 0.45f, 0.75f, m_SunDirection.x,
                                 m_SunDirection.y, m_SunDirection.z);

  m_Camera.SetGroundHeightFunction(
      [this](float x, float z) { return m_Terrain.GetHeight(x, z); });
  // Keep the walker out of buildings and inside the generated area
  m_Camera.SetCollisionFunction(
      [this](const glm::vec3 &, const glm::vec3 &to) {
        glm::vec2 ground =
            m_Buildings.Resolve(glm::vec2(to.x, to.z), kWalkerRadius);
        ground.x = std::clamp(ground.x, -m_HalfExtents.x, m_HalfExtents.x);
        ground.y = std::clamp(ground.y, -m_HalfExtents.y, m_HalfExtents.y);
        return glm::vec3(ground.x, to.y, ground.y);
      });
  m_Camera.SetEyeHeight(kEyeHeight);
  m_Camera.SetRunMultiplier(kRunMultiplier);
  m_Camera.SetMode(Sald::Camera::Mode::Walk);
  // The spawn point can sit on a wall (e.g. a path ending at a door)
  glm::vec2 spawn =
      m_Buildings.Resolve(glm::vec2(m_Spawn.x, m_Spawn.z), kWalkerRadius);
  m_Camera.SetPosition(glm::vec3(spawn.x, 0.0f, spawn.y));
  // Face the centre of the area, where the town is
  if (glm::length(spawn) > 0.01f)
    m_Camera.SetRotation(glm::degrees(std::atan2(-spawn.y, -spawn.x)), 0.0f);

  SetCursorCaptured(true);
}

void TownLayer::LoadSpawn(const std::string &path) {
  // "<spawn x> <spawn z> <half extent x> <half extent z>" (format in the README)
  std::ifstream file(path);
  float x = 0.0f, z = 0.0f, hx = 0.0f, hz = 0.0f;
  if (file >> x >> z >> hx >> hz) {
    m_Spawn = glm::vec3(x, 0.0f, z);
    m_HalfExtents = glm::vec2(hx, hz);
  } else {
    SALD_ERROR("Could not read spawn file {0}; starting at the origin", path);
    m_HalfExtents = glm::vec2(1.0e6f);
  }
}

void TownLayer::OnUpdate(Sald::Timestep timestep) {
  m_Camera.OnUpdate(timestep);

  glm::mat4 lightTransform = CalculateShadowTransform();
  ShadowPass(lightTransform);
  RenderPass(lightTransform);

  m_ShadersValidated = true;
}

glm::mat4 TownLayer::CalculateShadowTransform() {
  glm::vec3 player = m_Camera.GetCameraPosition();
  // Move the shadow area in whole shadow-map texels so shadow edges do not
  // crawl while walking
  float texel = 2.0f * kShadowRange / (float)kShadowMapSize;
  glm::vec3 center(std::floor(player.x / texel) * texel, 0.0f,
                   std::floor(player.z / texel) * texel);

  glm::mat4 projection = glm::ortho(-kShadowRange, kShadowRange, -kShadowRange,
                                    kShadowRange, 1.0f, 600.0f);
  glm::vec3 up = std::abs(m_SunDirection.y) > 0.99f
                     ? glm::vec3(0.0f, 0.0f, 1.0f)
                     : glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4 view = glm::lookAt(center - m_SunDirection * 300.0f, center, up);
  return projection * view;
}

void TownLayer::ShadowPass(const glm::mat4 &lightTransform) {
  Sald::Shader *shader = Sald::ShaderManager::GetShader("town_shadow");
  shader->Bind();
  m_Sun.GetShadowMap()->CreateFrameBuffer();

  glm::mat4 transform = lightTransform;
  shader->SetDirectionalLightTransform(&transform);
  shader->SetMat4("model", glm::mat4(1.0f));
  if (!m_ShadersValidated)
    shader->Validate();
  m_Town.Render();

  m_Sun.GetShadowMap()->DeleteFrameBuffer();
}

void TownLayer::UpdateProjection() {
  auto window = Sald::Application::GetInstance().GetWindow();
  int width = (int)window->GetBufferWidth();
  int height = (int)window->GetBufferHeight();
  if (width == m_ViewportWidth && height == m_ViewportHeight)
    return;
  m_ViewportWidth = width;
  m_ViewportHeight = height;
  if (width > 0 && height > 0)
    m_Projection = glm::perspective(glm::radians(60.0f),
                                    (float)width / (float)height, kNearPlane,
                                    kFarPlane);
}

void TownLayer::RenderPass(const glm::mat4 &lightTransform) {
  UpdateProjection();
  if (m_ViewportWidth <= 0 || m_ViewportHeight <= 0)
    return;

  Sald::RenderCommand::SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
  Sald::RenderCommand::SetClearColor(kSkyColor.r, kSkyColor.g, kSkyColor.b,
                                     1.0f);
  Sald::RenderCommand::Clear();

  Sald::Shader *shader = Sald::ShaderManager::GetShader("town");
  shader->Bind();

  glm::vec3 eye = m_Camera.GetCameraPosition();
  shader->SetMat4("projection", m_Projection);
  shader->SetMat4("view", m_Camera.CalculateViewMatrix());
  shader->SetFloat3("eyePosition", eye.x, eye.y, eye.z);
  shader->SetMat4("model", glm::mat4(1.0f));
  shader->SetMat4("normalMatrix", glm::mat4(1.0f));

  shader->SetFloat3("fogColor", kSkyColor.r, kSkyColor.g, kSkyColor.b);
  shader->SetFloat("fogDensity", kFogDensity);

  // Sun only: no point or spot lights in the town
  m_Sun.UseLight(shader);
  shader->SetInt("pointLightCount", 0);
  shader->SetInt("spotLightCount", 0);

  glm::mat4 transform = lightTransform;
  shader->SetDirectionalLightTransform(&transform);
  // Texture units: 1 object texture, 2 sun shadow map
  m_Sun.GetShadowMap()->Read(GL_TEXTURE2);
  shader->SetTexture(1);
  shader->SetDirectionalShadowMap(2);

  m_Material.UseMaterial(shader);
  if (!m_ShadersValidated)
    shader->Validate();
  m_Town.Render();
}

bool TownLayer::OnKeyPressed(Sald::KeyPressedEvent &event) {
  if (event.IsRepeat())
    return false;

  switch (event.GetKeyCode()) {
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

void TownLayer::OnEvent(Sald::Event &event) {
  Sald::EventDispatcher dispatcher(event);
  dispatcher.Dispatch<Sald::KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));
}

void TownLayer::SetCursorCaptured(bool captured) {
  m_CursorCaptured = captured;
  Sald::Application::GetInstance().GetWindow()->SetCursorEnabled(!captured);
  m_Camera.SetMouseLookEnabled(captured);
}
