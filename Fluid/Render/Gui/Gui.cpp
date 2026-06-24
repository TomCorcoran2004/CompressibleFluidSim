#include "Gui.h"
#include <vector>

#include <Imgui/imgui.h>
#include <OpenGLBase/Input/Input.h>
#include <OpenGlBase/Window/Window.h>
#include <glfw/glfw3.h>

namespace Gui
{
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_AlwaysAutoResize;
    bool WindowOpen = true;
    const char* WindowTitle = "Press F1 To Toggle";


    bool Init()
    {
        return true;
    }

    void Tick()
    {
        //handling Toggling Window
        using namespace Base::Input;
        if (WasKeyJustPressed(F1)) WindowOpen = !WindowOpen;

        if (WindowOpen == false) return;
        
        //Prevent Cropping Of Title
        f32 WindowTitleWidth = ImGui::CalcTextSize(WindowTitle).x;
        ImGui::SetNextWindowSizeConstraints( ImVec2(WindowTitleWidth + 50.f, 0), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Press F1 To Toggle", &WindowOpen, WindowFlags);
        
        if (ImGui::BeginTabBar("Tabs"))
        {
            if (ImGui::BeginTabItem("Program"))
            {
                if (ImGui::CollapsingHeader("Window"))
                {
                    Base::Window::CollapsingHeader();
                }
                if (ImGui::CollapsingHeader("Input"))
                {

                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Fluid"))
            {
                if (ImGui::CollapsingHeader("Visuals"))
                {

                }
                if (ImGui::CollapsingHeader("Config"))
                {
                    
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("DebugConsole"))
            {
                ImGui::Text("Debug Console");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void Destroy()
    {

    }
}