#include <iostream>
#include <sstream>
#include <string.h>
#include <application.h>
#include <imgui_extra_widget.h>
#include <ImGuiFileDialog.h>
#include <imgui_helper.h>
#include "ImMaskCreator.h"
#include "Morph.h"
#include "MatIo.h"

using namespace std;
using namespace ImGui;

static MaskCreator::Holder g_hMaskCreator;
static MaskCreator::Holder g_hMaskCreator2;
static bool g_bShowContainBox = false;
static bool g_bFillContour = true;
static ImMat g_mMask;
static ImTextureID g_tidMask = 0;
static int g_iMorphSize = 1;
static char g_acMaskSavePath[256];

// Application Framework Functions
static void _AppInitialize(void** handle)
{
    g_hMaskCreator = MaskCreator::CreateInstance();
    // g_hMaskCreator2 = MaskCreator::CreateInstance();
    strncpy(g_acMaskSavePath, "./mask.png", sizeof(g_acMaskSavePath));
}

static void _AppFinalize(void** handle)
{
    g_hMaskCreator = nullptr;
    g_hMaskCreator2 = nullptr;
}

static bool _AppFrame(void* handle, bool closeApp)
{
    bool quitApp = false;
    ImGuiIO& io = GetIO(); (void)io;
    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(io.DisplaySize, ImGuiCond_None);
    ImGui:Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    auto wndAvailSize = GetContentRegionAvail();
    if (BeginChild("left", {wndAvailSize.x/2, 0}, true))
    {
        TextUnformatted("Draw Mask Area");
        SameLine(0, 20);
        Checkbox("Show contain box", &g_bShowContainBox);
        SameLine(0, 20);
        if (Button("Load"))
        {
            const char *filters = "JSON文件(*.json){.JSON},.*";
            ImGuiFileDialog::Instance()->OpenDialog("LoadJsonFileDlgKey", ICON_IGFD_FOLDER_OPEN " 打开JSON文件", 
                                                    filters, "./", 1, nullptr, 
                                                    ImGuiFileDialogFlags_ShowBookmark | ImGuiFileDialogFlags_Modal);
        }
        SameLine(0, 20);
        if (Button("Save"))
        {
            const char *filters = "JSON文件(*.json){.JSON},.*";
            ImGuiFileDialog::Instance()->OpenDialog("SaveJsonFileDlgKey", ICON_IGFD_FOLDER_OPEN " 打开JSON文件", 
                                                    filters, "./", 1, nullptr, 
                                                    ImGuiFileDialogFlags_ShowBookmark | ImGuiFileDialogFlags_Modal);
        }

        ostringstream oss;
        oss << "Hovered point: ";
        auto pHoveredPoint = g_hMaskCreator->GetHoveredPoint();
        if (pHoveredPoint)
        {
            auto pos = pHoveredPoint->GetPos();
            oss << "pos(" << pos.x << ", " << pos.y << ")";
            auto off0 = pHoveredPoint->GetBezierGrabberOffset(0);
            oss << ", bg0(" << off0.x << ", " << off0.y << ")";
            auto off1 = pHoveredPoint->GetBezierGrabberOffset(1);
            oss << ", bg1(" << off1.x << ", " << off1.y << ")";
            auto hoverType = pHoveredPoint->GetHoverType();
            oss << ", ht=" << hoverType;
        }
        else
        {
            oss << "N/A";
        }
        string hoverPointInfo = oss.str();
        TextUnformatted(hoverPointInfo.c_str());

        wndAvailSize = GetContentRegionAvail();
        auto cursorPos = GetCursorScreenPos();
        if (!g_hMaskCreator->DrawContent(cursorPos, cursorPos+wndAvailSize))
            cerr << "MaskCreator::DrawContent() FAILED! Error is '" << g_hMaskCreator->GetError() << "'." << endl;
        if (g_bShowContainBox)
        {
            ImDrawList* pDrawList = GetWindowDrawList();
            auto _rContBox = g_hMaskCreator->GetContourContainBox();
            ImRect rContBox(_rContBox.x, _rContBox.y, _rContBox.z, _rContBox.w);
            static const ImU32 CONTBOX_COLOR = IM_COL32(250, 250, 250, 255);
            pDrawList->AddLine(rContBox.Min, {rContBox.Max.x, rContBox.Min.y}, CONTBOX_COLOR);
            pDrawList->AddLine({rContBox.Max.x, rContBox.Min.y}, rContBox.Max, CONTBOX_COLOR);
            pDrawList->AddLine(rContBox.Max, {rContBox.Min.x, rContBox.Max.y}, CONTBOX_COLOR);
            pDrawList->AddLine({rContBox.Min.x, rContBox.Max.y}, rContBox.Min, CONTBOX_COLOR);
        }
    }
    EndChild();

    SameLine();
    if (BeginChild("right", {0, 0}, true))
    {
        TextUnformatted("Show Mask Area");

        static const char* s_acLineTypeOpts[] = { "8-connect", "4-connect", "AA", "CapsuleSDF" };
        static const int s_iLintTypeOptsCnt = sizeof(s_acLineTypeOpts)/sizeof(s_acLineTypeOpts[0]);
        static int s_iLintTypeSelIdx = 0;
        PushItemWidth(200);
        if (BeginCombo("Line Type", s_acLineTypeOpts[s_iLintTypeSelIdx]))
        {
            for (auto i = 0; i < s_iLintTypeOptsCnt; i++)
            {
                const bool bSelected = i == s_iLintTypeSelIdx;
                if (Selectable(s_acLineTypeOpts[i], bSelected))
                    s_iLintTypeSelIdx = i;
                if (bSelected)
                    SetItemDefaultFocus();
            }
            EndCombo();
        }
        PopItemWidth();
        SameLine(0, 20);
        Checkbox("Fill contour", &g_bFillContour);

        g_mMask = g_hMaskCreator->GetMask(s_iLintTypeSelIdx, g_bFillContour);
        if (!g_mMask.empty())
            ImGenerateOrUpdateTexture(g_tidMask, g_mMask.w, g_mMask.h, g_mMask.c, (const unsigned char *)g_mMask.data);

        PushItemWidth(200);
        InputText("##SavePath", g_acMaskSavePath, sizeof(g_acMaskSavePath));
        PopItemWidth();
        SameLine(0, 5);
        BeginDisabled(g_mMask.empty());
        if (Button("Save"))
        {
            MatUtils::SaveAsPng(g_mMask, g_acMaskSavePath);
        }
        EndDisabled();

        ostringstream oss; oss << "Kernel size: (" << (g_iMorphSize*2+1) << "," << (g_iMorphSize*2+1) << ")";
        string strKsize = oss.str();
        TextUnformatted(strKsize.c_str());
        SameLine(0, 10);
        PushItemWidth(100);
        InputInt("Morph size", &g_iMorphSize);
        PopItemWidth();
        SameLine(0, 10);
        if (Button("Erode") && !g_mMask.empty())
        {
            MatUtils::Size2i szKsize(g_iMorphSize*2+1, g_iMorphSize*2+1);
            MatUtils::Point2i ptAnchor(g_iMorphSize, g_iMorphSize);
            ImMat mKernel = MatUtils::GetStructuringElement(MatUtils::MORPH_RECT, szKsize, ptAnchor);
            g_mMask = MatUtils::Erode(g_mMask, mKernel, ptAnchor);
            ImGenerateOrUpdateTexture(g_tidMask, g_mMask.w, g_mMask.h, g_mMask.c, (const unsigned char *)g_mMask.data);
        }
        SameLine(0, 10);
        if (Button("Dilate") && !g_mMask.empty())
        {
            MatUtils::Size2i szKsize(g_iMorphSize*2+1, g_iMorphSize*2+1);
            MatUtils::Point2i ptAnchor(g_iMorphSize, g_iMorphSize);
            ImMat mKernel = MatUtils::GetStructuringElement(MatUtils::MORPH_RECT, szKsize, ptAnchor);
            g_mMask = MatUtils::Dilate(g_mMask, mKernel, ptAnchor);
            ImGenerateOrUpdateTexture(g_tidMask, g_mMask.w, g_mMask.h, g_mMask.c, (const unsigned char *)g_mMask.data);
        }

        auto currPos = GetCursorScreenPos();
        wndAvailSize = GetContentRegionAvail();
        GetWindowDrawList()->AddRectFilled(currPos, currPos+wndAvailSize, IM_COL32_WHITE);
        if (g_tidMask)
            Image(g_tidMask, wndAvailSize);
    }
    EndChild();

    End();

    // open file dialog
    ImVec2 modal_center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImVec2 maxSize = ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y);
	ImVec2 minSize = maxSize * 0.5f;
    if (ImGuiFileDialog::Instance()->Display("LoadJsonFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
	{
        if (ImGuiFileDialog::Instance()->IsOk())
		{
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            g_hMaskCreator = MaskCreator::LoadFromJson(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("SaveJsonFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
	{
        if (ImGuiFileDialog::Instance()->IsOk())
		{
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            g_hMaskCreator->SaveAsJson(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (closeApp)
        quitApp = true;
    return quitApp;
}

void Application_Setup(ApplicationWindowProperty& property)
{
    property.name = "ImMaskCreator Test";
    property.viewport = false;
    property.docking = false;
    property.auto_merge = false;
    property.width = 1280;
    property.height = 720;
    property.application.Application_Initialize = _AppInitialize;
    property.application.Application_Finalize = _AppFinalize;
    property.application.Application_Frame = _AppFrame;
}
