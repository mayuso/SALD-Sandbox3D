#pragma once
#include "Sald/Core/Application.h"

class SandboxApp : public Sald::Application
{
public:
    // Runs the town walk when Models/town.obj exists, the demo scene otherwise
    // (or always, with forceDemo)
    explicit SandboxApp(bool forceDemo = false);
};
