#include <OpenGlBase/Base.h>
#include <OpenGlBase/Window/Window.h>
#include <OpenGlBase/Input/Input.h>
#include <OpenGLBase/Debug/Log.h>

#include "Fluid/Render/Render.h"
#include "Fluid/Sim/Mesh.h"
#include "Fluid/Sim/Fluid.h"
#include "Fluid/Sim/Scenes/SodShock.h"


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
    
    SodShock1D SodShockScene = {  };
    SodShockScene.Init(1000.f, 0.00001f);

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
