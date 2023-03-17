#include "imgui.h"
#include "imgui_helper.h"
#include "imgui_impl_glfw.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include "application.h"
#if IMGUI_VULKAN_SHADER
#include <ImVulkanShader.h>
#endif
#include "entry_vulkan.h"

#define GLFW_VERSION_COMBINED           (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)
#define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorWorkarea

static ApplicationWindowProperty property;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void DropCallback(GLFWwindow*, int count, const char** paths)
{
    std::vector<std::string> file_paths;
    for (int i = 0; i < count; i++)
    {
        file_paths.push_back(paths[i]);
    }
    if (!file_paths.empty() && property.application.Application_DropFromSystem)
    {
        property.application.Application_DropFromSystem(file_paths);
    }
}

void Application_FullScreen(bool on)
{
    ImGui_ImplGlfw_FullScreen(ImGui::GetMainViewport(), on);
}

int main(int argc, char** argv)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    property.argc = argc;
    property.argv = argv;
    Application_Setup(property);

    ImVec2 display_scale = ImVec2(1.0, 1.0);

    std::string title = property.name;
    title += " Vulkan GLFW";
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (property.resizable) glfwWindowHint(GLFW_RESIZABLE, true);
    else glfwWindowHint(GLFW_RESIZABLE, false);
    if (property.full_size || property.full_screen)
    {
        GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode * mode = glfwGetVideoMode(pMonitor);
        int x = 0, y = 0, w = 0, h = 0;
#if GLFW_HAS_MONITOR_WORK_AREA
        glfwGetMonitorWorkarea(pMonitor, &x, &y, &w, &h);
#endif
        property.pos_x = (x > 0 && x < 100) ? x : x + FULLSCREEN_OFFSET_X;
        property.pos_y = y + FULLSCREEN_OFFSET_Y;
        property.width = mode->width - FULLSCREEN_WIDTH_ADJ;
        property.height = mode->height - y;
        property.center = false;
        glfwWindowHint(GLFW_DECORATED, false);
    }
    else
    {
        if (property.top_most)
        {
            glfwWindowHint(GLFW_FLOATING, true);
        }
        if (!property.window_border)
        {
            glfwWindowHint(GLFW_DECORATED, false);
        }
    }

    GLFWwindow* window = glfwCreateWindow(property.width, property.height, title.c_str(), NULL, NULL);
    if (!window)
    {
        printf("GLFW: Create Main window Error!!!\n");
        return 1;
    }

    // Set window icon
    if (!property.icon_path.empty())
    {
        ImGui_ImplGlfw_SetWindowIcon(window, property.icon_path.c_str());
    }

#if !defined(__APPLE__) && GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >=3
    float x_scale, y_scale;
    glfwGetWindowContentScale(window, &x_scale, &y_scale);
    if (x_scale != 1.0 || y_scale != 1.0)
    {
        property.scale = x_scale == 1.0 ? x_scale : y_scale;
        display_scale = ImVec2(x_scale, y_scale);
    }
#endif
    glfwSetDropCallback(window, DropCallback);
    
    // Setup Vulkan
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }
    if (!property.center)
    {
        glfwSetWindowPos(window, property.pos_x, property.pos_y);
    }

    uint32_t extensions_count = 0;
    const char** ext = glfwGetRequiredInstanceExtensions(&extensions_count);
    std::vector<const char*> extensions;
    for (int i = 0; i < extensions_count; i++)
        extensions.push_back(ext[i]);
    SetupVulkan(extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
    check_vk_result(err);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    auto ctx = ImGui::CreateContext();
    if (property.application.Application_SetupContext)
        property.application.Application_SetupContext(ctx);

    // Create Framebuffers
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault(property.font_scale);
    io.FontGlobalScale = 1.0f / property.font_scale;
    io.DisplayFramebufferScale = display_scale;
    if (property.power_save) io.ConfigFlags |= ImGuiConfigFlags_EnableLowRefreshMode;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    if (property.docking) io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    if (property.viewport)io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    if (!property.auto_merge) io.ConfigViewportsNoAutoMerge = true;
    // Setup App setting file path
    auto setting_path = property.using_setting_path ? ImGuiHelper::settings_path(property.name) : "";
    auto ini_name = property.name;
    std::replace(ini_name.begin(), ini_name.end(), ' ', '_');
    setting_path += ini_name + ".ini";
    io.IniFilename = setting_path.c_str();
    auto language_path = property.language_path + ini_name + "_language.ini";
    if (property.internationalize)
    {
        io.LanguageFileName = language_path.c_str();
        g.Style.TextInternationalize = 1;
        g.LanguageName = "Default";
    }
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Allocator = g_Allocator;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.CheckVkResultFn = check_vk_result;
    // Setup ImGui binding
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    UpdateVulkanFont(wd);

#if IMGUI_VULKAN_SHADER
    ImGui::ImVulkanShaderInit();
#endif

    if (property.application.Application_Initialize)
        property.application.Application_Initialize(&property.handle);

    // Main loop
    bool done = false;
    bool splash_done = false;
    bool app_done = false;
    while (!app_done)
    {
        ImGui_ImplGlfw_WaitForEvent();
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            done = true;
        // Resize swap chain?
        if (g_SwapChainRebuild)
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
                g_MainWindowData.FrameIndex = 0;
                g_SwapChainRebuild = false;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (io.ConfigFlags & ImGuiConfigFlags_EnableLowRefreshMode)
            ImGui::SetMaxWaitBeforeNextFrame(1.0 / property.fps);

        if (property.application.Application_SplashScreen)
        {
            splash_done = property.application.Application_SplashScreen(property.handle, done);
        }
        else
            splash_done = true;
        
        if (splash_done && property.application.Application_Frame)
            app_done = property.application.Application_Frame(property.handle, done);
        else
            app_done = done;

        ImGui::EndFrame();
        // Rendering
        ImGui::Render();
        FrameRendering(wd);
    }

    if (property.application.Application_Finalize)
        property.application.Application_Finalize(&property.handle);

    // Cleanup
#if IMGUI_VULKAN_SHADER
    ImGui::ImVulkanShaderClear();
#endif
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
