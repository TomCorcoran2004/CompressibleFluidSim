#include "Window.h"
#include <cassert>
#include <glfw/glfw3.h>
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include <ImGui/Helpers.h>

#include "../Debug/Log.h"
#include "MonitorHelper.h"

namespace Base
{    
    namespace Window
    {
        GLFWwindow* Handle = nullptr;

        ivec2 WindowedPosition = {  };
        ivec2 FullScreenPosition = {  };
        bool FullScreen = false;

        bool PendingClose = false;

        ivec2 FrameBufferSize = { 0, 0 };
        ivec2 Size = { 0, 0 };

        f64 DeltaTime = 0.0;
        f64 LastFrameTime = 0.0;

        ivec2 LastWindowedSize = { 0, 0 };
        ivec2 LastWindowedPosition = { 0, 0 };

        static void PositionCallBack(GLFWwindow* WindowInstance, int x, int y)
        {
            if (glfwGetWindowMonitor(WindowInstance) == nullptr)
            {
                WindowedPosition = ivec2(x, y);
            }
            else
            {
                FullScreenPosition = ivec2(x, y);
            }
        }

        static void SizeCallBack(GLFWwindow* WindowInstance, int Width, int Height)
        {
            Size = ivec2(Width, Height);
        }

        static void FrameBufferSizeCallBack(GLFWwindow* WindowInstance, int Width, int Height)
        {
            glViewport(0, 0, Width, Height);
            FrameBufferSize = ivec2(Width, Height);
        }

        bool Init(const Config& Config)
        {            
            //setup initual Monitor
            MonitorHelper::Init();

            //Set Window Hints
            glfwDefaultWindowHints();

            glfwWindowHint(GLFW_RESIZABLE, Config.Resizeable);
            glfwWindowHint(GLFW_VISIBLE, Config.InitiallyVisible);
            glfwWindowHint(GLFW_DECORATED, Config.HaveDecorations);
            glfwWindowHint(GLFW_FOCUSED, Config.InituiallyFocused);
            glfwWindowHint(GLFW_CENTER_CURSOR, Config.CenterCursorOnStartup);

            //Creating a window
            FullScreen = Config.FullScreen;
            GLFWmonitor* Monitor = FullScreen ? MonitorHelper::GetCurrentMonitor().GetHandle() : nullptr;
            Handle = glfwCreateWindow(Config.Size.x, Config.Size.y, Config.Title, Monitor, nullptr);
            if (Handle == nullptr)
            {
                Log::Error("GLFWWindow* WindowInstance == nullptr");
                return false;
            }

            glfwMakeContextCurrent(Handle);

            bool GladInitSuccess = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
            if (GladInitSuccess == false)
            {
                Log::Error("GladInitSuccess == false");
                return false;
            }

            //Setting Up Callbacks for info about window
            glfwSetFramebufferSizeCallback(Handle, FrameBufferSizeCallBack);
            glfwSetWindowSizeCallback(Handle, SizeCallBack);
            glfwSetWindowPosCallback(Handle, PositionCallBack);

            //Initializing All Values To Avoid Leaving Them Un-Initialised Until First CallBacks
            glfwGetFramebufferSize(Handle, &FrameBufferSize.x, &FrameBufferSize.y);
            glfwGetWindowSize(Handle, &Size.x, &Size.y);
            
            if (FullScreen) glfwGetWindowPos(Handle, &FullScreenPosition.x, &FullScreenPosition.y);
            if (FullScreen == false) glfwGetWindowPos(Handle, &WindowedPosition.x, &WindowedPosition.y);

            //need to sort vsync handling
            glfwSwapInterval(0); //disable vsync

            glViewport(0, 0, Config.Size.x, Config.Size.y);

            return true;
        }

        void Destroy()
        {
            assert(Handle);
            glfwDestroyWindow(Handle);
        }

        bool ShouldClose()
        {
            assert(Handle);
            return glfwWindowShouldClose(Handle);
        }

        void Tick()
        {
            assert(Handle);

            if (PendingClose) glfwSetWindowShouldClose(Handle, true);

            f64 CurrentTime = glfwGetTime();
            DeltaTime = CurrentTime - LastFrameTime;
            LastFrameTime = CurrentTime;

            glfwPollEvents();
            glfwSwapBuffers(Handle);
        }

        ivec2 GetWindowPos() { return glfwGetWindowMonitor(Handle) ? WindowedPosition : FullScreenPosition; }
        ivec2 GetWindowSize() { return Size; }
        ivec2 GetFrameBufferSize() { return FrameBufferSize; }
        GLFWwindow* GetGLFWWindow() { return Handle; }
        f64 GetDeltaTime() { return DeltaTime; }
        
        void Close() { PendingClose = true; }

        void CollapsingHeader()
        {
            std::vector<std::string> MonitorNames = MonitorHelper::GetMonitorNames();
            i32 CurrentMonitorIndex = MonitorHelper::GetCurrentMonitorIndex();

            if (ImGui::ComboBoxHelper("Monitor", MonitorNames, CurrentMonitorIndex))
            {
                MonitorHelper::SetCurrentMonitor(CurrentMonitorIndex);
            }

            MonitorHelper::Monitor& CurrentMonitor = MonitorHelper::GetCurrentMonitor();
            std::vector<std::string> VideoModesStr = CurrentMonitor.GetFormattedVideoModes();
            i32 CurrentVideoMode = CurrentMonitor.GetCurrentVideoModeIndex();
            bool VideoModeUpdated = false;

            if (ImGui::ComboBoxHelper("Resolutions", VideoModesStr, CurrentVideoMode))
            {
                CurrentMonitor.SetVideoMode(CurrentVideoMode);
                VideoModeUpdated = true;
            }

            if (ImGui::Checkbox("Fullscreen", &FullScreen) || VideoModeUpdated)
            {
                const GLFWvidmode VideoMode = CurrentMonitor.GetCurrentVideoMode();
                
                if (FullScreen == true)
                {
                    glfwSetWindowMonitor(Handle, CurrentMonitor.GetHandle(), 0, 0, VideoMode.width, VideoMode.height, VideoMode.refreshRate);
                }
                else if (FullScreen == false)
                {
                    glfwSetWindowMonitor(Handle, nullptr, WindowedPosition.x, WindowedPosition.y, VideoMode.width, VideoMode.height, VideoMode.refreshRate);
                }
            }
        }
    }
}