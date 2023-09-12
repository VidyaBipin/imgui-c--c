#include "ImGuiTabWindow.h"
#include <string>
namespace ImGui
{
void ShowAddonsTabWindow()
{
    ImGui::Spacing();
    ImGui::Text("TabLabels (based on the code by krys-spectralpixel):");
    static const std::vector<std::string> tabNames = {"TabLabelStyle","Render","Layers","Scene","World","Object","Constraints","Modifiers","Data","Material","Texture","Particle"};
    static const std::vector<std::string> tabTooltips = {"Edit the style of these labels","Render Tab Tooltip","This tab cannot be closed","Scene Tab Tooltip","","Object Tab Tooltip","","","","","Tired to add tooltips..."};
    static int tabItemOrdering[] = {0,1,2,3,4,5,6,7,8,9,10,11};
    static int selectedTab = 0;
    static int optionalHoveredTab = 0;
    static bool allowTabLabelDragAndDrop=true;
    static bool tabLabelWrapMode = true;
    static bool allowClosingTabs = false;
    static bool tableAtBottom = false;
    int justClosedTabIndex=-1,justClosedTabIndexInsideTabItemOrdering = -1,oldSelectedTab = selectedTab;

    ImGui::Checkbox("Wrap Mode##TabLabelWrapMode",&tabLabelWrapMode);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","WrapMode is only available\nin horizontal TabLabels");
    ImGui::SameLine();ImGui::Checkbox("Drag And Drop##TabLabelDragAndDrop",&allowTabLabelDragAndDrop);
    ImGui::SameLine();ImGui::Checkbox("Closable##TabLabelClosing",&allowClosingTabs);ImGui::SameLine();
    ImGui::Checkbox("At bottom##TabLabelAtbottom",&tableAtBottom);
    bool resetTabLabels = ImGui::SmallButton("Reset Tabs");if (resetTabLabels) {selectedTab=0;for (int i=0;i<tabNames.size();i++) tabItemOrdering[i] = i;}

    ImVec2 table_size;
    if (!tableAtBottom)
    {
        ImGui::TabLabels(tabNames,selectedTab,table_size,tabTooltips,tabLabelWrapMode,false,&optionalHoveredTab,&tabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,&justClosedTabIndex,&justClosedTabIndexInsideTabItemOrdering);
        // Optional stuff
        if (justClosedTabIndex==1) {
            tabItemOrdering[justClosedTabIndexInsideTabItemOrdering] = justClosedTabIndex;   // Prevent the user from closing Tab "Layers"
            selectedTab = oldSelectedTab;   // This is safer, in case we had closed the selected tab
        }
    }
    // Draw tab page
    ImGui::BeginChild("MyTabLabelsChild",ImVec2(0,150),true);
    ImGui::Text("Tab Page For Tab: \"%s\" here.", selectedTab >= 0 ? tabNames[selectedTab].c_str() : "None!");
    if (selectedTab==0) {
        static bool editTheme = false;
        ImGui::Spacing();
        ImGui::Checkbox("Edit tab label style",&editTheme);
        ImGui::Spacing();
        if (editTheme) ImGui::TabLabelStyle::Edit(ImGui::TabLabelStyle().Get());   // This is good if we want to edit the tab label style
        else {
            static int selectedIndex=0;
            ImGui::PushItemWidth(135);
            ImGui::SelectTabLabelStyleCombo("select tab label style",&selectedIndex); // Good for just selecting it
            ImGui::PopItemWidth();
        }
    }
    ImGui::EndChild();
    if (tableAtBottom)
    {
        ImGui::TabLabels(tabNames,selectedTab,table_size,tabTooltips,tabLabelWrapMode,false,&optionalHoveredTab,&tabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,&justClosedTabIndex,&justClosedTabIndexInsideTabItemOrdering, true);
        // Optional stuff
        if (justClosedTabIndex==1) {
            tabItemOrdering[justClosedTabIndexInsideTabItemOrdering] = justClosedTabIndex;   // Prevent the user from closing Tab "Layers"
            selectedTab = oldSelectedTab;   // This is safer, in case we had closed the selected tab
        }
    }

    // ImGui::TabLabelsVertical() are similiar to ImGui::TabLabels(), but they do not support WrapMode.
    // ImGui::TabLabelsVertical() example usage
    static bool verticalTabLabelsAtLeft = true;ImGui::Checkbox("Vertical Tab Labels at the left side##VerticalTabLabelPosition",&verticalTabLabelsAtLeft);
    static const std::vector<std::string> verticalTabNames = {"Layers","Scene","World"};
    static const std::vector<std::string> verticalTabTooltips = {"Layers Tab Tooltip","Scene Tab Tooltip","World Tab Tooltip"};
    static int verticalTabItemOrdering[3] = {0,1,2};
    static int selectedVerticalTab = 0;
    static int optionalHoveredVerticalTab = 0;
    if (resetTabLabels) {selectedVerticalTab=0;for (int i=0;i<verticalTabNames.size();i++) verticalTabItemOrdering[i] = i;}

    const float verticalTabsWidth = ImGui::CalcVerticalTabLabelsWidth();
    if (verticalTabLabelsAtLeft)	{
        ImGui::TabLabelsVertical(verticalTabNames,selectedVerticalTab,verticalTabTooltips,false,&optionalHoveredVerticalTab,&verticalTabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,NULL,NULL,false,verticalTabLabelsAtLeft);
        ImGui::SameLine(0,0);
    }
    // Draw tab page
    ImGui::BeginChild("MyVerticalTabLabelsChild",ImVec2(ImGui::GetWindowWidth()-verticalTabsWidth-2.f*ImGui::GetStyle().WindowPadding.x-ImGui::GetStyle().ScrollbarSize,150),true);
    ImGui::Text("Tab Page For Tab: \"%s\" here.",selectedVerticalTab >= 0 ? verticalTabNames[selectedVerticalTab].c_str() : "None!");
    ImGui::EndChild();
    if (!verticalTabLabelsAtLeft)	{
        ImGui::SameLine(0,0);
        ImGui::TabLabelsVertical(verticalTabNames,selectedVerticalTab,verticalTabTooltips,false,&optionalHoveredVerticalTab,&verticalTabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,NULL,NULL,false,verticalTabLabelsAtLeft);
    }
}
} // namespace ImGui