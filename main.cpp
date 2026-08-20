#include <OpenGlBase/Base.h>
#include <OpenGlBase/Window/Window.h>
#include <OpenGlBase/Input/Input.h>
#include <OpenGLBase/Debug/Log.h>

#include "Fluid/Render/Render.h"
#include "Fluid/Sim/Mesh/Mesh.h"
#include "Fluid/Sim/Fluid/Fluid.h"
#include "Fluid/Sim/Scene/SodShock1D/SodShock.h"

#include "Fluid/Sim/Scene/SodShock1D/RiemannSolver/Riemann.h"


int main()
{
    Base::Init();
    
    Base::Window::Config Config{
        .Size = glm::ivec2{800, 800},
    };

    Base::Window::Init(Config);
    Base::Input::Init();
    Render::Init();
    
    SodShock1D::Config SodShockConfig = {
        .NumCells = 10000,
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
