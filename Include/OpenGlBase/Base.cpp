#include "Base.h"
#include "Debug/Log.h"
#include <glfw/glfw3.h>

namespace Base
{
    bool Init()
    {
        if (glfwInit() == false)
        {
            Log::LastGLFWError();
            return false;
        }
        return true;
    }

    void Destroy()
    {
        glfwTerminate();
    }
};