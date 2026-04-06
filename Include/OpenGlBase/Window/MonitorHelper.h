#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct GLFWvidmode;
struct GLFWmonitor;

namespace Base
{
    namespace MonitorHelper
    {
        class Monitor
        {
        public:
            Monitor(GLFWmonitor* _Handle);
            GLFWmonitor* GetHandle() const;
            
            void SetVideoMode(i32 Index);
            const GLFWvidmode& GetVideoMode(i32 Index) const;
            const GLFWvidmode& GetCurrentVideoMode() const;
            i32 GetCurrentVideoModeIndex() const;
            
            const std::vector<GLFWvidmode>& GetVideoModes() const;
            std::vector<std::string> GetFormattedVideoModes() const;
            
            bool operator==(const Monitor& Other) const;

        private:
            GLFWmonitor* Handle = nullptr;
            std::vector<GLFWvidmode> VideoModes;
            i32 CurrentVideoModeIndex;
        };

        i32 GetCurrentMonitorIndex();
        i32 GetMonitorIndex(GLFWmonitor* Monitor);
        Monitor& GetCurrentMonitor();
        Monitor& GetMonitor(i32 Index);
        void SetCurrentMonitor(i32 Index);
        bool Init();
        std::vector<std::string> GetMonitorNames();
    }
}

