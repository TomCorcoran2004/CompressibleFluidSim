#include "Window.h"
#include <glfw/glfw3.h>
#include <glad/glad.h>

#include "../Debug/Log.h"

namespace Base
{    
    namespace Window
    {
        void SetWindowHints(const Config& Config);

        void SetPositionInternal(const ivec2& NewPosition);
        void SetSizeInternal(const ivec2& NewSize);
        void SetFrameBufferSizeInternal(const ivec2& NewFrameBufferSize);

        static void PositionCallBack(GLFWwindow* WindowInstance, int X, int Y);
        static void SizeCallBack(GLFWwindow* WindowInstance, int Width, int Height);
        static void FrameBufferSizeCallBack(GLFWwindow* WindowInstance, int Width, int Height);
        
        GLFWwindow* WindowInstance = nullptr;
        GLFWmonitor* Monitor = nullptr;
        bool PendingClose = false;

        ivec2 FrameBufferSize = { 0, 0 };
        ivec2 Size = { 0, 0 };
        ivec2 Position = { 0, 0 };

        f64 DeltaTime = 0.0;
        f64 LastFrameTime = 0.0;

        ivec2 LastWindowedSize = { 0, 0 };
        ivec2 LastWindowedPosition = { 0, 0 };

        bool Init(const Config& Config)
        {
            assert(Config.Title);

            Monitor = Config.Monitor ? Config.Monitor : glfwGetPrimaryMonitor();
            SetWindowHints(Config);

            WindowInstance = glfwCreateWindow(Config.Size.x, Config.Size.y, Config.Title, Config.Monitor, nullptr);

            if (WindowInstance == nullptr)
            {
                Log::Error("GLFWWindow* WindowInstance == nullptr");
                assert(WindowInstance);
                return false;
            }

            glfwMakeContextCurrent(WindowInstance);
            bool GladInitSuccess = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
            if (GladInitSuccess == false)
            {
                Log::Error("GladInitSuccess == false");
                assert(GladInitSuccess);
                return false;
            }

            glfwSetFramebufferSizeCallback(WindowInstance, FrameBufferSizeCallBack);
            glfwSetWindowSizeCallback(WindowInstance, SizeCallBack);
            glfwSetWindowPosCallback(WindowInstance, PositionCallBack);

            //Initializing All Values To Avoid Leaving Them Un-Initialised Until First CallBacks
            glfwGetFramebufferSize(WindowInstance, &FrameBufferSize.x, &FrameBufferSize.y);
            glfwGetWindowSize(WindowInstance, &Size.x, &Size.y);
            glfwGetWindowPos(WindowInstance, &Position.x, &Position.y);

            glfwSwapInterval(0); //disable vsync

            glViewport(0, 0, Config.Size.x, Config.Size.y);
        }

        void Destroy()
        {
            assert(WindowInstance);
            glfwDestroyWindow(WindowInstance);
        }

        bool ShouldClose()
        {
            assert(WindowInstance);
            return glfwWindowShouldClose(WindowInstance);
        }

        void Tick()
        {
            assert(WindowInstance);

            if (PendingClose) glfwSetWindowShouldClose(WindowInstance, true);

            f64 CurrentTime = glfwGetTime();
            DeltaTime = CurrentTime - LastFrameTime;
            LastFrameTime = CurrentTime;

            glfwPollEvents();
            glfwSwapBuffers(WindowInstance);
        }

        ivec2 GetWindowPos()
        {
            return Position;
        }

        ivec2 GetWindowSize()
        {
            return Size;
        }

        ivec2 GetFrameBufferSize()
        {
            return FrameBufferSize;
        }

        GLFWwindow* GetGLFWWindow()
        {
            return WindowInstance;
        }

        GLFWmonitor* GetPrimaryMonitor()
        {
            return Monitor;
        }

        f64 GetDeltaTime()
        {
            return DeltaTime;
        }

        void Close()
        {
            PendingClose = true;
        }

        void SetWindowPos(const ivec2& NewPosition)
        {
            assert(WindowInstance);

            glfwSetWindowPos(WindowInstance, Position.x, Position.y);
        }

        void SetWindowSize(const ivec2& Size)
        {
            assert(WindowInstance);

            glfwSetWindowSize(WindowInstance, Size.x, Size.y);
        }

        void ToggleFullscreen()
        {
            assert(WindowInstance);
            assert(Monitor);

            if (IsFullScreen())
            {
                glfwSetWindowMonitor(WindowInstance, nullptr, LastWindowedPosition.x, LastWindowedPosition.y, LastWindowedSize.x, LastWindowedSize.y, 0);
            }
            else
            {
                LastWindowedPosition = GetWindowPos();
                LastWindowedSize = GetWindowSize();

                const GLFWvidmode* VideoMode = glfwGetVideoMode(Monitor);

                glfwSetWindowMonitor(WindowInstance, Monitor, 0, 0, VideoMode->width, VideoMode->height, VideoMode->refreshRate);
            }
        }

        void Minimize()
        {
            assert(WindowInstance);
            glfwIconifyWindow(WindowInstance);
        }

        bool IsFullScreen()
        {
            assert(WindowInstance);

            return glfwGetWindowMonitor(WindowInstance);
        }

        void SetWindowHints(const Config& Config)
        {
            glfwDefaultWindowHints();

            glfwWindowHint(GLFW_RESIZABLE, Config.Resizeable);
            glfwWindowHint(GLFW_VISIBLE, Config.InitiallyVisible);
            glfwWindowHint(GLFW_DECORATED, Config.HaveDecorations);
            glfwWindowHint(GLFW_FOCUSED, Config.InituiallyFocused);
            glfwWindowHint(GLFW_CENTER_CURSOR, Config.CenterCursorOnStartup);
        }

        void SetSizeInternal(const ivec2& NewSize)
        {
            Size = NewSize;
        }
        void SetPositionInternal(const ivec2& NewPosition)
        {
            Position = NewPosition;
        }
        void SetFrameBufferSizeInternal(const ivec2& NewFrameBufferSize)
        {
            FrameBufferSize = NewFrameBufferSize;
        }

        void PositionCallBack(GLFWwindow* WindowInstance, int X, int Y)
        {
           SetPositionInternal(ivec2{ X, Y });
        }

        void SizeCallBack(GLFWwindow* WindowInstance, int Width, int Height)
        {
            SetSizeInternal(ivec2{ Width, Height });
        }

        void FrameBufferSizeCallBack(GLFWwindow* WindowInstance, int Width, int Height)
        {
            glViewport(0, 0, Width, Height);

            SetFrameBufferSizeInternal(ivec2{ Width, Height });
        }
    }
}