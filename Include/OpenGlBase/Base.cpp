#include "Base.h"
#include "Debug/Log.h"
#include <GLFW/glfw3.h>

namespace Base
{
    void Init()
    {
        if (glfwInit() == false)
        {
            Log::LastGLFWError();
        }
    }

    void Destroy()
    {
        glfwTerminate();
    }
};