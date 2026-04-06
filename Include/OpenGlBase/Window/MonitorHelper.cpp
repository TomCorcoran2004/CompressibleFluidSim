#include "MonitorHelper.h"
#include <algorithm>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>


namespace Base
{
    namespace MonitorHelper
    {        
        Monitor::Monitor(GLFWmonitor* _Handle)
        {
            Handle = _Handle;
                
            i32 NumVideoModes = 0;
            const GLFWvidmode* VideoModeArray = glfwGetVideoModes(Handle, &NumVideoModes);
            VideoModes.assign(VideoModeArray, VideoModeArray + NumVideoModes);
                
            const GLFWvidmode* CurrentVidMode = glfwGetVideoMode(Handle);

            auto Condition = [CurrentVidMode](const GLFWvidmode& vm) -> bool
            {
                return vm.width == CurrentVidMode->width &&
                    vm.height == CurrentVidMode->height &&
                    vm.redBits == CurrentVidMode->redBits &&
                    vm.greenBits == CurrentVidMode->greenBits &&
                    vm.blueBits == CurrentVidMode->blueBits &&
                    vm.refreshRate == CurrentVidMode->refreshRate;
            };

            auto it = std::find_if(VideoModes.begin(), VideoModes.end(), Condition);
            CurrentVideoModeIndex = static_cast<i32>(it - VideoModes.begin());
        }

        GLFWmonitor* Monitor::GetHandle() const
        {
            return Handle;
        }

        void Monitor::SetVideoMode(i32 Index)
        {
            if (Index >= 0 && Index < VideoModes.size())
                CurrentVideoModeIndex = Index;
        }

        const GLFWvidmode& Monitor::GetVideoMode(i32 Index) const
        {
            return VideoModes[Index];
        }

        const GLFWvidmode& Monitor::GetCurrentVideoMode() const 
        {
            return VideoModes[CurrentVideoModeIndex];
        }

        i32 Monitor::GetCurrentVideoModeIndex() const
        {
            return CurrentVideoModeIndex;
        }

        const std::vector<GLFWvidmode>& Monitor::GetVideoModes() const 
        {
            return VideoModes;
        }

        std::vector<std::string> Monitor::GetFormattedVideoModes() const 
        {
            std::vector<std::string> VideoModesFormatted(VideoModes.size());

            for (i32 i = 0; i < VideoModes.size(); ++i)
            {
                std::string FormattedVideoMode = std::to_string(VideoModes[i].width) + "x" + std::to_string(VideoModes[i].height) + " @" + std::to_string(VideoModes[i].refreshRate);
                VideoModesFormatted[i] = FormattedVideoMode;
            }

            return VideoModesFormatted;
        }

        bool Monitor::operator==(const Monitor& Other) const
        {
            return Handle == Other.Handle;
        }

        std::vector<Monitor> Monitors;
        i32 CurrentMonitorIndex = 0;

        void MonitorCallBack(GLFWmonitor* Monitor, int Event)
        {
            if (Event == GLFW_CONNECTED)
            {
                Monitors.emplace_back(Monitor);
            }
            else if (Event == GLFW_DISCONNECTED)
            {
                Monitors.erase(std::find(Monitors.begin(), Monitors.end(), Monitor));
                if (Monitors[CurrentMonitorIndex] == Monitor)
                {
                    GLFWmonitor* Primary = glfwGetPrimaryMonitor();
                    auto it = std::find(Monitors.begin(), Monitors.end(), Primary);

                    CurrentMonitorIndex = static_cast<i32>(it - Monitors.begin());
                }
            }
        }

        i32 GetCurrentMonitorIndex()
        {
            return CurrentMonitorIndex;
        }

        i32 GetMonitorIndex(GLFWmonitor* Monitor)
        {
            auto it = std::find(Monitors.begin(), Monitors.end(), Monitor);
            if (it != Monitors.end())
                return static_cast<i32>(it - Monitors.begin());
            else
                return -1;
        }

        Monitor& GetCurrentMonitor()
        {
            return Monitors[CurrentMonitorIndex];
        }

        Monitor& GetMonitor(i32 Index)
        {
            return Monitors[Index];;
        }

        void SetCurrentMonitor(i32 Index)
        {
            if (Index >= 0 && Index < Monitors.size())
                CurrentMonitorIndex = Index;
        }

        bool Init()
        {
            i32 NumMonitors = 0; 
            GLFWmonitor** MonitorsArray = glfwGetMonitors(&NumMonitors);
            Monitors.assign(MonitorsArray, MonitorsArray + NumMonitors);
            CurrentMonitorIndex = GetMonitorIndex(glfwGetPrimaryMonitor());

            return true;
        }

        std::vector<std::string> GetMonitorNames()
        {
            std::vector<std::string> MonitorNames(Monitors.size());

            for (i32 i = 0; i < Monitors.size(); ++i)
            {
                MonitorNames[i] = glfwGetMonitorName(Monitors[i].GetHandle());
                MonitorNames[i] += "##" + std::to_string(i);
            }

            return MonitorNames;
        }
    }
}