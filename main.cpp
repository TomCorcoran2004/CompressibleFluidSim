#include <OpenGlBase/Base.h>
#include <OpenGlBase/Window/Window.h>
#include <OpenGlBase/Input/Input.h>
#include <OpenGLBase/Debug/Log.h>

#include "Fluid/Render/Render.h"

int main()
{
    Base::Init();
    
    Base::Window::Config Config{
        .Size = glm::ivec2{800, 800},
    };
    Base::Window::Init(Config);
    
    Base::Input::Init();

    Render::Init();

    while (Base::Window::ShouldClose() == false)
    {
        Base::Window::Tick();
        Base::Input::Tick();

        Render::Tick();
    }

    Render::Destroy();
    Base::Input::Destroy();
    Base::Window::Destroy();
    Base::Destroy();
    return 0;
}
