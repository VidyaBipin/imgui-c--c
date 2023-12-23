#include <application.h>
#include <imgui.h>
#include <imgui_helper.h>
#include <imgui_extra_widget.h>
#include <imgui_json.h>
#include <implot.h>
#include <ImGuiFileDialog.h>
#include <ImGuiTabWindow.h>
#include <ImGuiZmo.h>
#include <imgui_orient.h>
#include <model.h>
#include <sstream>
#include <iomanip>
#include <getopt.h>

static std::string g_model_path = "";
static std::string g_language_path = "";
static std::string g_resource_path = "";
/****************************************************************************************
 * 
 * Application Framework
 *
 ***************************************************************************************/
/*
static bool App_Splash_Screen(void* handle, bool app_will_quit)
{
}

static void App_Splash_Finalize(void** handle)
{
}
*/

static void App_SetupContext(ImGuiContext* ctx, void* handle, bool in_splash)
{
    if (!ctx)
        return;
#ifdef USE_BOOKMARK
    ImGuiSettingsHandler bookmark_ini_handler;
    bookmark_ini_handler.TypeName = "BookMark";
    bookmark_ini_handler.TypeHash = ImHashStr("BookMark");
    bookmark_ini_handler.ReadOpenFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) -> void*
    {
        return ImGuiFileDialog::Instance();
    };
    bookmark_ini_handler.ReadLineFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) -> void
    {
        IGFD::FileDialog * dialog = (IGFD::FileDialog *)entry;
        if (dialog) dialog->DeserializeBookmarks(line);
    };
    bookmark_ini_handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
    {
        ImGuiContext& g = *ctx;
        out_buf->reserve(out_buf->size() + g.SettingsWindows.size() * 6); // ballpark reserve
        auto bookmark = ImGuiFileDialog::Instance()->SerializeBookmarks();
        out_buf->appendf("[%s][##%s]\n", handler->TypeName, handler->TypeName);
        out_buf->appendf("%s\n", bookmark.c_str());
        out_buf->append("\n");
    };
    ctx->SettingsHandlers.push_back(bookmark_ini_handler);
#endif
}

static void App_Initialize(void** handle)
{
    ImPlot::CreateContext();
}

static void App_Finalize(void** handle)
{
    ImPlot::DestroyContext();
}

static void App_DropFromSystem(std::vector<std::string>& drops)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern | ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_AcceptPeekOnly))
	{
		ImGui::EndDragDropSource();
	}
}

static bool App_Frame(void * handle, bool app_will_quit)
{
    static bool app_done = false;
    static bool inited = false;
    auto& io = ImGui::GetIO();
    bool multiviewport = io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    ImGuiCond cond = ImGuiCond_Once;
    ImVec2 Canvas_size;
    if (multiviewport)
    {
        io.ConfigViewportsNoDecoration = false;
        flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | 
                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
        Canvas_size = viewport->WorkSize;
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
    }
    else
    {
        flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | 
                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
        Canvas_size = io.DisplaySize;
        cond = ImGuiCond_None;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize, cond);
    }
    ImGui::Begin("Content", nullptr, flags);

    // background ImGuizmo
    static float cameraView[16] =
    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    static const float identityMatrix[16] =
    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    static float cubeMatrix[16] =
    {   1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    static float cameraProjection[16];
    static bool bFace = true;
    static bool bMesh = false;
    static bool bNormal = false;
    static float fov = 30.f;
    static float camYAngle = ImDegToRad(165.f);
    static float camXAngle = ImDegToRad(15.f);
    static float camDistance = 8.f;
    static float camShiftX = 0.0f;
    static float camShiftY = 0.0f;
    static std::vector<Model*> models;

    bool bControlHoverd = false;
    bool bAnyPopup = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
    ImGuizmo::Perspective(fov, Canvas_size.x / Canvas_size.y, 0.1f, 100.f, cameraProjection, camShiftX, camShiftY);
    ImGuizmo::SetOrthographic(false);

    auto cameraViewUpdate = [&]()
    {
        ImGuizmo::Perspective(fov, Canvas_size.x / Canvas_size.y, 0.1f, 100.f, cameraProjection, camShiftX, camShiftY);
        float eye[] = { cosf(camYAngle) * cosf(camXAngle) * camDistance, sinf(camXAngle) * camDistance, sinf(camYAngle) * cosf(camXAngle) * camDistance };
        float at[] = { 0.f, 0.f, 0.f };
        float up[] = { 0.f, 1.f, 0.f };
        ImGuizmo::LookAt(eye, at, up, cameraView);
        for (auto model : models)
            ImGuizmo::UpdateModel(cameraView, cameraProjection, model);
    };

    if (!inited)
    {
        cameraViewUpdate();
        inited = true;
    }

    // draw zmo
    ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, Canvas_size.x, Canvas_size.y);
    ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);

    // draw models
    for (auto model : models)
    {
        // Draw Bodel
        ImGuizmo::DrawModel(cameraView, cameraProjection, model, bFace, bMesh, bNormal);

        if (model->showManipulate)
        {
            bool updated = ImGuizmo::Manipulate(cameraView, cameraProjection, model->currentGizmoOperation, model->currentGizmoMode, (float *)&model->identity_matrix, NULL, 
                                                model->useSnap ? (float *)&model->snap : NULL, 
                                                model->boundSizing ? (float *)&model->bounds : NULL, 
                                                model->boundSizingSnap ? (float *)&model->boundsSnap : NULL);
            if (updated)
            {
                ImGuizmo::UpdateModel(cameraView, cameraProjection, model);
            }
            bControlHoverd |= updated;
        }
    }

    // draw view Manipulate
    float viewManipulateRight = ImGui::GetWindowPos().x + Canvas_size.x;
    float viewManipulateTop = ImGui::GetWindowPos().y;
    if (ImGuizmo::ViewManipulate(cameraView, camDistance, ImVec2(viewManipulateRight - 256, viewManipulateTop + 64), ImVec2(128, 128), 0x10101010, &camXAngle, &camYAngle))
    {
        cameraViewUpdate();
        bControlHoverd = true;
    }
    ImGui::PopStyleColor();

    // draw control panel
    ImGuiWindowFlags control_window_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        const ImGuiViewport* viewport = ImGui::GetWindowViewport();
        control_window_flags |= ImGuiWindowFlags_NoDocking;
        io.ConfigViewportsNoDecoration = true;
        ImGui::SetNextWindowViewport(viewport->ID);
    }
    ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0.5, 0.5, 0.5));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5, 0.5, 0.5, 1.0));
    if (ImGui::Begin("Model Control", nullptr, control_window_flags))
    {
        bControlHoverd |= ImGui::IsWindowFocused();
        if (ImGui::Button(ICON_FK_REFRESH " Reset View"))
        {
            camYAngle = ImDegToRad(165.f);
            camXAngle = ImDegToRad(15.f);
            camDistance = 8.f;
            camShiftX = 0.0f;
            camShiftY = 0.0f;
            cameraViewUpdate();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Face", &bFace);
        ImGui::SameLine();
        ImGui::Checkbox("Mesh", &bMesh);
        ImGui::SameLine();
        ImGui::Checkbox("Normal", &bNormal);
        if (ImGui::Button(ICON_FK_PLUS " Add model"))
        {
            ImGuiFileDialog::Instance()->OpenDialog("##OpenFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose OBJ File", 
                                                    "3D Object file(*.obj){.obj}",
                                                    ".",
                                                    1, 
                                                    IGFDUserDatas("Open Model"), 
                                                    ImGuiFileDialogFlags_ShowBookmark | ImGuiFileDialogFlags_CaseInsensitiveExtention | ImGuiFileDialogFlags_DisableCreateDirectoryButton | ImGuiFileDialogFlags_Modal);
        }

        for (auto iter = models.begin(); iter != models.end();)
        {
            Model * model = *iter;
            std::string model_label = "Model #" + ImGuiHelper::MillisecToString(model->model_id, 1);
            auto model_label_id = ImGui::GetID(model_label.c_str());
            ImGui::PushID(model_label_id);
            if (ImGui::TreeNodeEx(model_label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowOverlap))
            {
                if (ImGui::RadioButton("Translate", model->currentGizmoOperation == ImGuizmo::TRANSLATE))
                    model->currentGizmoOperation = ImGuizmo::TRANSLATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate", model->currentGizmoOperation == ImGuizmo::ROTATE))
                    model->currentGizmoOperation = ImGuizmo::ROTATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Scale", model->currentGizmoOperation == ImGuizmo::SCALE))
                    model->currentGizmoOperation = ImGuizmo::SCALE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Universal", model->currentGizmoOperation == ImGuizmo::UNIVERSAL))
                    model->currentGizmoOperation = ImGuizmo::UNIVERSAL;
                ImGuizmo::DecomposeMatrixToComponents(model->identity_matrix, model->matrixTranslation, model->matrixRotation, model->matrixScale);
                ImGui::PushItemWidth(240);
                ImGui::InputFloat3("Tr", (float*)&model->matrixTranslation);
                ImGui::InputFloat3("Rt", (float*)&model->matrixRotation);
                ImGui::InputFloat3("Sc", (float*)&model->matrixScale);
                ImGuizmo::RecomposeMatrixFromComponents(model->matrixTranslation, model->matrixRotation, model->matrixScale, model->identity_matrix);
                ImGui::PopItemWidth();
                if (model->currentGizmoOperation != ImGuizmo::SCALE)
                {
                    if (ImGui::RadioButton("Local", model->currentGizmoMode == ImGuizmo::LOCAL))
                        model->currentGizmoMode = ImGuizmo::LOCAL;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("World",model->currentGizmoMode == ImGuizmo::WORLD))
                        model->currentGizmoMode = ImGuizmo::WORLD;
                }
                ImGui::Separator();
                ImGui::Checkbox("Manipulate", &model->showManipulate);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FK_HOME " Reset"))
                {
                    ImGuizmo::DecomposeMatrixToComponents(identityMatrix, model->matrixTranslation, model->matrixRotation, model->matrixScale);
                    model->matrixScale *= 0.2f;
                    ImGuizmo::RecomposeMatrixFromComponents(model->matrixTranslation, model->matrixRotation, model->matrixScale, model->identity_matrix);
                    ImGuizmo::UpdateModel(cameraView, cameraProjection, model);
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FK_TRASH_O " Delete"))
                {
                    delete model;
                    iter = models.erase(iter);
                }
                else
                    iter++;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImVec2 minSize = ImVec2(600, 600);
        ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);
        if (multiviewport) ImGui::SetNextWindowViewport(viewport->ID);
        if (ImGuiFileDialog::Instance()->Display("##OpenFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                auto file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                auto file_name = ImGuiFileDialog::Instance()->GetCurrentFileName();
                auto userDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
                if (userDatas.compare("Open Model") == 0)
                {
                    Model* model = new Model();
                    model->model_data = LoadObj(file_path);
                    if (model->model_data->model_open)
                    {
                        model->model_id = ImGui::get_current_time_usec();
                        ImGuizmo::DecomposeMatrixToComponents(model->identity_matrix, model->matrixTranslation, model->matrixRotation, model->matrixScale);
                        model->matrixScale *= 0.2f;
                        ImGuizmo::RecomposeMatrixFromComponents(model->matrixTranslation, model->matrixRotation, model->matrixScale, model->identity_matrix);
                        ImGuizmo::UpdateModel(cameraView, cameraProjection, model);
                        models.emplace_back(model);
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        io.ConfigViewportsNoDecoration = false;
    }

    // handle mouse wheel
    if (!bControlHoverd && !bAnyPopup)
    {
        if (io.MouseWheelH < -FLT_EPSILON)
        {
            camYAngle += ImDegToRad(2.f);
            cameraViewUpdate();
        }
        else if (io.MouseWheelH > FLT_EPSILON)
        {
            camYAngle -= ImDegToRad(2.f);
            cameraViewUpdate();
        }
        else if (io.MouseWheel < -FLT_EPSILON)
        {
            camDistance *= 0.95;
            if (camDistance < 1.f) camDistance = 1.f;
            cameraViewUpdate();
        }
        else if (io.MouseWheel > FLT_EPSILON)
        {
            camDistance *= 1.05;
            if (camDistance > 10.f) camDistance = 10.f;
            cameraViewUpdate();
        }
        
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            camShiftX -= io.MouseDelta.x / Canvas_size.x / camDistance;
            camShiftY += io.MouseDelta.y / Canvas_size.y / camDistance;
            cameraViewUpdate();
        }
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << ImGui::GetIO().DeltaTime * 1000.f << "ms/frame ";
    oss << "(" << ImGui::GetIO().FrameCountSinceLastUpdate << "/" << ImGui::GetIO().FrameCountSinceLastEvent <<  ") ";
    oss << ImGui::GetIO().Framerate << "FPS";
    oss << " T:" << ImGui::ImGetTextureCount();
    oss << " V:" << io.MetricsRenderVertices;
    oss << " I:" << io.MetricsRenderIndices;
    oss << " F:" << fov;
    oss << " AX:" << ImRadToDeg(camXAngle);
    oss << " AY:" << ImRadToDeg(camYAngle);
    oss << " OX:" << camShiftX;
    oss << " OY:" << camShiftY; 
    std::string meters = oss.str();
    auto str_size = ImGui::CalcTextSize(meters.c_str());
    auto spos = ImGui::GetWindowPos() + ImVec2(10, Canvas_size.y - str_size.y - 16);
    ImGui::GetWindowDrawList()->AddText(spos, IM_COL32(255,255,255,128), meters.c_str());
    ImGui::End();
    if (app_will_quit)
    {
        for (auto model : models) delete model;
    }
    return app_done || app_will_quit;
}

void Application_Setup(ApplicationWindowProperty& property)
{
    // param commandline args
    static struct option long_options[] = {
        { "model_dir", required_argument, NULL, 'm' },
        { "language_dir", required_argument, NULL, 'l' },
        { "resuorce_dir", required_argument, NULL, 'r' },
        { 0, 0, 0, 0 }
    };
    if (property.argc > 1 && property.argv)
    {
        int o = -1;
        int option_index = 0;
        while ((o = getopt_long(property.argc, property.argv, "m:l:r:", long_options, &option_index)) != -1)
        {
            if (o == -1)
                break;
            switch (o)
            {
                case 'm': g_model_path = std::string(optarg); break;
                case 'l': g_language_path = std::string(optarg); break;
                case 'r': g_resource_path = std::string(optarg); break;
                default: break;
            }
        }
    }

    auto exec_path = ImGuiHelper::exec_path();
    // add language
    property.language_path = !g_language_path.empty() ? g_language_path : 
#if defined(__APPLE__)
        exec_path + "../Resources/languages/";
#elif defined(_WIN32)
        exec_path + "../languages/";
#elif defined(__linux__)
        exec_path + "../languages/";
#else
        std::string();
#endif
/*
    icon_file = 
    property.icon_path =  !g_resource_path.empty() ? g_resource_path + "/mec_logo.png" : 
#if defined(__APPLE__)
        exec_path + "../Resources/mec_logo.png";
#elif defined(__linux__)
        //exec_path + "mec.png";
        exec_path + "../resources/mec_logo.png";
#elif defined(_WIN32)
        exec_path + "../resources/mec_logo.png";
#else
        std::string();
#endif
*/
    property.name = "Obj Viewer";
    //property.viewport = false;
    property.docking = false;
    property.auto_merge = false;
    property.internationalize = true;
    property.navigator = false;
    //property.using_setting_path = false;
    property.low_reflash = true;
    property.power_save = true;
    property.font_scale = 2.0f;
#if 1
    //property.resizable = false;
    //property.full_size = true;
    //property.full_screen = true;
#else
    property.width = DEFAULT_MAIN_VIEW_WIDTH;
    property.height = DEFAULT_MAIN_VIEW_HEIGHT;
#endif
    //property.splash_screen_width = 800;
    //property.splash_screen_height = 400;
    //property.splash_screen_alpha = 0.95;
    //property.application.Application_SplashScreen = App_Splash_Screen;
    //property.application.Application_SplashFinalize = App_Splash_Finalize;
    property.application.Application_SetupContext = App_SetupContext;
    property.application.Application_Initialize = App_Initialize;
    property.application.Application_Finalize = App_Finalize;
    property.application.Application_DropFromSystem = App_DropFromSystem;
    property.application.Application_Frame = App_Frame;

    if (g_model_path.empty())
        g_model_path = ImGuiHelper::path_parent(exec_path) + "models";
}
