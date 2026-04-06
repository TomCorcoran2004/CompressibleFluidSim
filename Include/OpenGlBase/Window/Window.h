#pragma once
#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;
struct GLFWmonitor;
struct GLFWvidmode;

namespace Base
{    
    namespace Window
    {
        struct Config
        {
            uvec2 Pos = { 0, 0 };
            uvec2 Size = { 0, 0 };

            //Can All Be Left Default
            bool FullScreen = false;
            const char* Title = "Default Title";
            bool Resizeable = true;
            bool InitiallyVisible = true;
            bool HaveDecorations = true;
            bool InituiallyFocused = true;
            bool CenterCursorOnStartup = false;
        };

        bool Init(const Config& Config);
        void Destroy();

        bool ShouldClose();
        void Tick();

        ivec2 GetWindowPos();
        ivec2 GetWindowSize();
        ivec2 GetFrameBufferSize();
        GLFWwindow* GetGLFWWindow();
        
        void Close();
        f64 GetDeltaTime();
        
        void CollapsingHeader();
    }
}