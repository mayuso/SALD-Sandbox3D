#include "SandboxApp.h"

#include <cstring>

int main(int argc, char **argv)
{
    // `sald-sandbox3d demo` shows the lighting demo even when a town model exists
    bool forceDemo = argc > 1 && std::strcmp(argv[1], "demo") == 0;

    // On the stack so ~Application runs: it frees the layers and GL resources
    // and terminates GLFW
    SandboxApp sandboxApp(forceDemo);
    sandboxApp.Run();
    return 0;
}
