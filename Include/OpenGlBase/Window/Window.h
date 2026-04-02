#pragma once
#include <glm/glm.hpp>

struct GLFWwindow;
struct GLFWmonitor;

namespace Base
{    
    namespace Window
    {
        struct Hint
        {
            i32 Hint;
            i32 Value;
        };

        struct Config
        {
            uvec2 Pos = { 0, 0 };
            uvec2 Size = { 0, 0 };
            GLFWmonitor* Monitor = nullptr;

            //Can All Be Left Default
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
        GLFWmonitor* GetPrimaryMonitor();
        f64 GetDeltaTime();

        void Close();
        void SetWindowPos(const ivec2& NewPosition);
        void SetWindowSize(const ivec2& Size);

        void ToggleFullscreen();
        bool IsFullScreen();
        void Minimize();
    }
}