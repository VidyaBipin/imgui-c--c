#include "ImNewCurve.h"

void ImGui::ImNewCurve::ShowDemo()
{
    ImGuiIO& io = ImGui::GetIO();
    static bool bUseTimebase = false;
    static int aTimebaseNumDen[] = {1, 25};
    static bool scroll_v = true;
    static bool move_curve = true;
    static bool keep_begin_end = false;
    static bool dock_begin_end = false;
    static ImGui::ImNewCurve::Editor::Holder s_hCurveEditor;

    float table_width = 300;
    auto size_x = ImGui::GetWindowSize().x - table_width - 60;

    if (!s_hCurveEditor)
    {
        s_hCurveEditor =  ImGui::ImNewCurve::Editor::CreateInstance(ImVec2(0, size_x));
        s_hCurveEditor->SetBackgroundColor(IM_COL32(0, 0, 0, 255));
        s_hCurveEditor->SetBackgroundColor(IM_COL32(32, 32, 32, 128));
        auto hCurve = s_hCurveEditor->AddCurveByDim("key1", ImGui::ImNewCurve::Smooth, ImGui::ImNewCurve::DIM_X, -1, 1, 0, IM_COL32(255, 0, 0, 255), true);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.f,     0),         ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.25f,   0.610f),    ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.5f,    1.0f),      ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.75f,   0.610f),    ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*1.f,     0.f),       ImGui::ImNewCurve::Smooth, false);
        hCurve = s_hCurveEditor->AddCurveByDim("key2", ImGui::ImNewCurve::Smooth, ImGui::ImNewCurve::DIM_X, 0, 1, 0, IM_COL32(0, 255, 0, 255), true);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.f,     1.f),       ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.25f,   0.75f),     ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.5f,    0.5f),      ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.75f,   0.75f),     ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*1.f,     1.f),       ImGui::ImNewCurve::Smooth, false);
        hCurve = s_hCurveEditor->AddCurveByDim("key3", ImGui::ImNewCurve::Smooth, ImGui::ImNewCurve::DIM_X, 0, 100, 50, IM_COL32(0, 0, 255, 255), true);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.f,     0.f),       ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.25f,   0.05f),     ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.5f,    0.25f),     ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*0.75f,   0.75f),     ImGui::ImNewCurve::Smooth, false);
        hCurve->AddPointByDim(ImGui::ImNewCurve::DIM_X, ImVec2(size_x*1.f,     1.f),       ImGui::ImNewCurve::Smooth, false);
        s_hCurveEditor->SetShowValueToolTip(true);
    }
    const auto& curve_type_list = ImGui::ImNewCurve::GetCurveTypeNames();
    bool reset = false;
    if (ImGui::Button("Reset##curve_reset"))
        reset = true;
    if (s_hCurveEditor->GetTimeRange().y <= 0 || reset)
        s_hCurveEditor->SetTimeRange(ImVec2(0, size_x), false);
    if (ImGui::Checkbox("Use Timebase", &bUseTimebase))
        s_hCurveEditor = nullptr;
    ImGui::SameLine();
    ImGui::BeginDisabled(!bUseTimebase);
    if (ImGui::InputInt2("##Timebase", aTimebaseNumDen))
    {
        if (aTimebaseNumDen[0] <= 0) aTimebaseNumDen[0] = 1;
        if (aTimebaseNumDen[1] <= 0) aTimebaseNumDen[1] = 1;
    }
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::Checkbox("Scroll V", &scroll_v); ImGui::SameLine();
    ImGui::Checkbox("Move Curve", &move_curve); ImGui::SameLine();
    ImGui::Checkbox("Keep Begin End", &keep_begin_end); ImGui::SameLine();
    ImGui::Checkbox("Dock Begin End", &dock_begin_end);
    s_hCurveEditor->DrawDim(ImGui::ImNewCurve::DIM_X, "##curve_editor_view", ImVec2(size_x, 300), 0);
}
