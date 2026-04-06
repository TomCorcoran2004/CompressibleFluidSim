#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace ImGui
{
    bool ComboBoxHelper(const char* Title, const std::vector<const char*>& Items, i32& CurrentItem);
    bool ComboBoxHelper(const char* Title, const std::vector<std::string>& Items, i32& CurrentItem);
}


