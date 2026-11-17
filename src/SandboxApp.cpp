#include "SandboxApp.h"
#include "SandboxLayer.h"
#include "TownLayer.h"

#include <filesystem>

namespace {
// Bundled town asset; the demo scene is used when it is missing
constexpr const char *kTownName = "town";
} // namespace

SandboxApp::SandboxApp(bool forceDemo) : Sald::Application(1280, 720, Sald::Application::Dimensions::Three)
{
    std::string townModel = std::string("Models/") + kTownName + ".obj";
    if (!forceDemo && std::filesystem::exists(townModel))
        PushLayer(new TownLayer(kTownName));
    else
        PushLayer(new SandboxLayer());
}
