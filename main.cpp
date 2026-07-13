#include <OpenGlBase/Base.h>
#include <OpenGlBase/Window/Window.h>
#include <OpenGlBase/Input/Input.h>
#include <OpenGLBase/Debug/Log.h>

#include "Fluid/Render/Render.h"
#include "Fluid/Sim/Mesh/Mesh.h"
#include "Fluid/Sim/Fluid/Fluid.h"
#include "Fluid/Sim/Scene/SodShock1D/SodShock.h"

int main()
{
    Base::Init();
    
    Base::Window::Config Config{
        .Size = glm::ivec2{800, 800},
    };
    Base::Window::Init(Config);
    
    Base::Input::Init();

    Fluid::Config FluidConfig = Fluid::Config{
        .dt = 1.0f / 100.f,
        .R = 1.0f,
        .gamma = 1.4f
    };

    Render::Init();
    
    SodShock1D::Config SodShockConfig = {
        .NumCells = 10000,
        .TotalTicks = 20000
    };

    SodShock1D SodShockScene = { SodShockConfig };

    while (Base::Window::ShouldClose() == false)
    {
        Base::Window::Tick();
        Base::Input::Tick();
        
        Render::BeginFrame();

        SodShockScene.Tick();

        Render::Render();

        Render::EndFrame();
    }

    Render::Destroy();
    Base::Input::Destroy();
    Base::Window::Destroy();
    Base::Destroy();
    return 0;
}
