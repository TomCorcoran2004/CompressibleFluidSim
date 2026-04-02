#include "Render.h"
#include <Glad/glad.h>
#include <OpenGlBase/Window/Window.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

namespace Render
{
    bool Init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(Base::Window::GetGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 130");

        return true;
    }

    void Tick()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Render();
        
        ivec2 FrameBufferSize = Base::Window::GetFrameBufferSize();
        glViewport(0, 0, FrameBufferSize.x, FrameBufferSize.y);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Destroy()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}