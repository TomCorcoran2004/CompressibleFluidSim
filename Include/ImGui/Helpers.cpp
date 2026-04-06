#include "Helpers.h"
#include "ImGui/imgui.h"
namespace ImGui
{
    bool ComboBoxHelper(const char* Title, const std::vector<const char*>& Items, i32& CurrentItem)
    {
        if (Items.empty() == true)
        {
            //Todo: Log To Console
            return false;
        }

        if (CurrentItem < 0 || CurrentItem >= Items.size())
        {
            //Todo: Log To Console
            return false;
        }

        bool CurrentItemUpdated = false;

        if (ImGui::BeginCombo(Title, Items[CurrentItem]))
        {
            for (i32 i = 0; i < Items.size(); ++i)
            {
                bool IsSelected = CurrentItem == i;
                if (ImGui::Selectable(Items[i], IsSelected))
                {
                    CurrentItem = i;
                    CurrentItemUpdated = true;
                }
                if (IsSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        return CurrentItemUpdated;
    }

    bool ComboBoxHelper(const char* Title, const std::vector<std::string>& Items, i32& CurrentItem)
    {
        if (Items.empty() == true)
        {
            //Todo: Log To Console
            return false;
        }

        if (CurrentItem < 0 || CurrentItem >= Items.size())
        {
            //Todo: Log To Console
            return false;
        }

        std::vector<const char*> ItemsChar(Items.size());
        for (i32 i = 0; i < Items.size(); ++i)
        {
            ItemsChar[i] = Items[i].data();
        }

        bool CurrentItemUpdated = false;

        if (ImGui::BeginCombo(Title, ItemsChar[CurrentItem]))
        {
            for (i32 i = 0; i < ItemsChar.size(); ++i)
            {
                bool IsSelected = CurrentItem == i;
                if (ImGui::Selectable(ItemsChar[i], IsSelected))
                {
                    CurrentItem = i;
                    CurrentItemUpdated = true;
                }
                if (IsSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        return CurrentItemUpdated;
    }
}