#include "Render.h"
#include <Glad/glad.h>
#include <OpenGlBase/Window/Window.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/implot.h>

#include "Gui/Gui.h"

namespace Render
{
    bool Init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(Base::Window::GetGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 130");

        if (Gui::Init() == false)
        {
            //TODO: Error Handling
            return false;
        }

        return true;
    }

    void BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Render()
    {
        Gui::Tick();
    }

    void EndFrame()
    {
        ImGui::Render();

        ivec2 FrameBufferSize = Base::Window::GetFrameBufferSize();
        glViewport(0, 0, FrameBufferSize.x, FrameBufferSize.y);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Destroy()
    {
        Gui::Destroy();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }
}