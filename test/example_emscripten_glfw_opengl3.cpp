#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_extra_widget.h>
#include <implot.h>
#include <imgui_markdown.h>
#include <imgui_memory_editor.h>
#include <ImGuiFileDialog.h>
#include <HotKey.h>
#include <TextEditor.h>
#include <ImGuiTabWindow.h>
#include <imgui_node_editor.h>
#include <imgui_curve.h>
#include <imgui_spline.h>
#include <ImGuiZMOquat.h>
#include <ImGuiZmo.h>
#include <imgui_toggle.h>
#include <imgui_tex_inspect.h>
#include <portable-file-dialogs.h>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <fstream>
#include <sstream>
#include <string>

#include <emscripten.h>
#include <functional>
static std::function<void()>            MainLoopForEmscriptenP;
static void MainLoopForEmscripten()     { MainLoopForEmscriptenP(); }
#define EMSCRIPTEN_MAINLOOP_BEGIN       MainLoopForEmscriptenP = [&]()
#define EMSCRIPTEN_MAINLOOP_END         ; emscripten_set_main_loop(MainLoopForEmscripten, 0, true)

static ImGui::ImMat image(256, 256, 4, 1u, 4);
static ImGui::ImMat draw_mat {ImGui::ImMat(512, 512, 4, 1u, 4)};
static ImTextureID ImageTexture = 0;
static ImTextureID DrawMatTexture = 0;

static std::vector<ImHotKey::HotKey> hotkeys = 
{ 
    {"Layout", "Reorder nodes in a simpler layout", 0xFFFF26E0},
    {"Save", "Save the current graph", 0xFFFF1FE0},
    {"Load", "Load an existing graph file", 0xFFFF18E0},
    {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
    {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
};

static ImGuiFileDialog filedialog;

// Init Colorful Text Edit
static TextEditor editor;

int8_t data[0x1000];
static MemoryEditor mem_edit;

static ImGui::MarkdownConfig mdConfig; 
static std::string get_file_contents()
{
    return "Dear ImGui \n" \
        "===== \n" \
        "[![Build Status](https://github.com/ocornut/imgui/workflows/build/badge.svg)](https://github.com/ocornut/imgui/actions?workflow=build) [![Static Analysis Status](https://github.com/ocornut/imgui/workflows/static-analysis/badge.svg)](https://github.com/ocornut/imgui/actions?workflow=static-analysis) \n" \
        "<sub>(This library is available under a free and permissive license, but needs financial support to sustain its continued improvements. In addition to maintenance and stability there are many desirable features yet to be added. If your company is using Dear ImGui, please consider reaching out.)</sub> \n" \
        "Businesses: support continued development and maintenance via invoiced technical support, maintenance, sponsoring contracts:" \
        "<br>&nbsp;&nbsp;_E-mail: contact @ dearimgui dot com_ \n" \
        "Individuals: support continued development and maintenance [here](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=WGHNC6MBFLZ2S). \n" \
        "Also see [Sponsors](https://github.com/ocornut/imgui/wiki/Sponsors) page. \n";
}

inline ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    // In your application you would load an image based on data_ input. Here we just use the imgui font texture.
    ImTextureID image = ImGui::GetIO().Fonts->TexID;
    // > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };
    ImGui::MarkdownImageData imageData;
    imageData.isValid =         true;
    imageData.useLinkCallback = false;
    imageData.user_texture_id = image;
    imageData.size =            ImVec2( 40.0f, 20.0f );
    return imageData;
}

static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    std::string url( data_.link, data_.linkLength );
    std::string command = "open " + url;
    if( !data_.isImage )
    {
        system(command.c_str());
    }
}

static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
{
    // Call the default first so any settings can be overwritten by our implementation.
    // Alternatively could be called or not called in a switch statement on a case by case basis.
    // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
    ImGui::defaultMarkdownFormatCallback( markdownFormatInfo_, start_ );        
    switch( markdownFormatInfo_.type )
    {
        // example: change the colour of heading level 2
        case ImGui::MarkdownFormatType::HEADING:
        {
            if( markdownFormatInfo_.level == 2 )
            {
                if( start_ )
                {
                    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] );
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

static inline void box(ImGui::ImMat& image, int x1, int y1, int x2, int y2, int R, int G, int B)
{
    for (int j = y1; j <= y2; j++)
    {
        for (int i = x1; i <= x2; i++)
        {
            //unsigned int color = 0xFF000000 | (R << 16) | (G << 8) | B;
            //image.at<unsigned int>(i, j) = color;
            image.at<unsigned char>(i, j, 3) = 0xFF;
            image.at<unsigned char>(i, j, 2) = B;
            image.at<unsigned char>(i, j, 1) = G;
            image.at<unsigned char>(i, j, 0) = R;
        }
    }
}

static inline void color_bar(ImGui::ImMat& image, int x1, int y1, int x2, int y2)
{
    const unsigned char r[8] = {255,255,0,0,255,255,0,0};
    const unsigned char g[8] = {255,255,255,255,0,0,0,0};
    const unsigned char b[8] = {255,0,255,0,255,0,255,0};
    int len = x2 - x1 + 1;
    for (int i = 0; i < 8; i++)
    {
        box(image, x1 + len * i / 8, y1, x1 + len * (i + 1) / 8 - 1, y2, r[i], g[i], b[i]);
    }
}

static inline void gray_bar(ImGui::ImMat& image, int x1,int y1,int x2,int y2,int step)
{
    int len = x2 - x1 + 1;
    for (int i = 0; i < step; i++)
    {
        box(image, x1 + len * i / step, y1, x1 + len * (i + 1) / step - 1, y2, 255 * i / step, 255 * i / step, 255 * i / step);
    }
}

static void DrawLineDemo()
{
    float t = (float)ImGui::GetTime();
    float h = abs(sin(t * 0.2));
    float s = abs(sin(t * 0.1)) * 0.5 + 0.4;
    float h2 = abs(sin(t * 0.4));
    ImVec4 base_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImVec4 light_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.5f, base_color.x, base_color.y, base_color.z);
    ImGui::ColorConvertHSVtoRGB(h2, s, 0.5f, light_color.x, light_color.y, light_color.z);
    static float arc = 0.0;
    draw_mat.clean(ImPixel(0.f, 0.f, 0.f, 1.f));
    arc += 2 * M_PI / 64 / 32;
    if (arc > 2 * M_PI / 64) arc = 0;
    float cx = draw_mat.w * 0.5f, cy = draw_mat.h * 0.5f;
    ImPixel line_color(base_color.x, base_color.y, base_color.z, 1.f);
    ImPixel circle_color(light_color.x, light_color.y, light_color.z, 1.f);

    // draw line test
    for (int j = 0; j < 5; j++) 
    {
        float r1 = fminf(draw_mat.w, draw_mat.h) * (j + 0.5f) * 0.085f;
        float r2 = fminf(draw_mat.w, draw_mat.h) * (j + 1.5f) * 0.085f;
        float t = j * M_PI / 64.0f, r = (j + 1) * 0.5f;
        for (int i = 1; i <= 64; i++, t += 2.0f * M_PI / 64.0f)
        {
            float ct = cosf(t + arc), st = sinf(t + arc);
            draw_mat.draw_line(ImPoint(cx + r1 * ct, cy - r1 * st), ImPoint(cx + r2 * ct, cy - r2 * st), r, line_color);
        }
    }

    // draw circle test(smooth) 
    for (int j = 0; j < 5; j++)
    {
        float r = fminf(draw_mat.w, draw_mat.h) * (j + 1.5f) * 0.085f + 1;
        float t = (j + 1) * 0.5f;
        draw_mat.draw_circle(draw_mat.w / 2, draw_mat.h / 2, r, t, circle_color);
    }

    // draw circle test
    draw_mat.draw_circle(draw_mat.w / 2, draw_mat.h / 2, draw_mat.w / 2 - 1, ImPixel(1.0, 1.0, 1.0, 1.0));

    ImGui::ImMatToTexture(draw_mat, DrawMatTexture);
    ImGui::Image(DrawMatTexture, ImVec2(draw_mat.w, draw_mat.h));
}

static void WarpMatrixDemo()
{
    const float width = 1920.f;
    const float height = 1080.f;
    ImVec2 src_corners[4];
    ImVec2 dst_corners[4];
    src_corners[0] = ImVec2(width / 1.80, height / 4.20);
    src_corners[1] = ImVec2(width / 1.15, height / 3.32);
    src_corners[2] = ImVec2(width / 1.33, height / 1.10);
    src_corners[3] = ImVec2(width / 1.93, height / 1.36);
    dst_corners[0] = ImVec2(0, 0);
    dst_corners[1] = ImVec2(width, 0);
    dst_corners[2] = ImVec2(width, height);
    dst_corners[3] = ImVec2(0, height);
    ImGui::ImMat M0 = ImGui::getPerspectiveTransform(dst_corners, src_corners);
    ImGui::ImMat M1 = ImGui::getAffineTransform(dst_corners, src_corners);
    for (int i = 0; i < 4; i++)
    {
        ImGui::Text("d: x=%.2f y=%.2f", dst_corners[i].x, dst_corners[i].y);
        ImGui::SameLine(200);
        ImGui::Text("s: x=%.2f y=%.2f", src_corners[i].x, src_corners[i].y);
    }
    ImGui::Separator();
    ImGui::TextUnformatted("Perspective Transform:");
    for (int h = 0; h < M0.h; h++)
    {
        for (int w = 0; w < M0.w; w++)
        {
            ImGui::Text("%.2f", M0.at<float>(w, h));
            if ( w <  M0.w - 1)
                ImGui::SameLine((w + 1) * 100);
        }
    }
    ImGui::TextUnformatted("Affine Transform:");
    for (int h = 0; h < M1.h; h++)
    {
        for (int w = 0; w < M1.w; w++)
        {
            ImGui::Text("%.2f", M1.at<float>(w, h));
            if ( w <  M1.w - 1)
                ImGui::SameLine((w + 1) * 100);
        }
    }
}

// Our state (make them static = more or less global) as a convenience to keep the example terse.
static bool show_demo_window = true;
static bool show_another_window = false;
static bool show_file_dialog_window = false;
static bool show_implot_window = false;
static bool show_markdown_window = false;
static bool show_widget_window = false;
static bool show_mat_draw_window = false;
static bool show_mat_warp_matrix = false;
static bool show_kalman_window = false;
static bool show_fft_window = false;
static bool show_stft_window = false;
static bool show_text_editor_window = false;
static bool show_tab_window = false;
static bool show_node_editor_window = false;
static bool show_curve_demo_window = false;
static bool show_spline_demo_window = false;
static bool show_zmoquat_window = false;
static bool show_zmo_window = false;
static bool show_toggle_window = false;
static bool show_tex_inspect_window = false;
static bool show_portable_file_dialogs = false;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 Emscripten example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.IniFilename = NULL;
    io.FontGlobalScale = 0.5;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

     // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Emscripten allows preloading a file or folder to be accessible at runtime. See Makefile for details.
    io.Fonts->AddFontDefault();

    prepare_file_dialog_demo_window(&filedialog, nullptr);

    mem_edit.Open = false;
    mem_edit.OptShowDataPreview = true;
    mem_edit.OptAddrDigitsCount = 8;

    color_bar(image, 0, 0, 255, 191);
    gray_bar(image, 0, 192, 255, 255, 13);
    ImageTexture = ImGui::ImCreateTexture(image.data, image.w, image.h);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    EMSCRIPTEN_MAINLOOP_BEGIN
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        
        // Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::Checkbox("File Dialog Window", &show_file_dialog_window);
            ImGui::Checkbox("Portable File Dialogs", &show_portable_file_dialogs);
            ImGui::Checkbox("Memory Edit Window", &mem_edit.Open);
            ImGui::Checkbox("Show Markdown Window", &show_markdown_window);
            ImGui::Checkbox("Show Extra Widget Window", &show_widget_window);
            ImGui::Checkbox("Show Kalman Window", &show_kalman_window);
            ImGui::Checkbox("Show FFT Window", &show_fft_window);
            ImGui::Checkbox("Show STFT Window", &show_stft_window);
            ImGui::Checkbox("Show ImMat Draw Window", &show_mat_draw_window);
            ImGui::Checkbox("Show ImMat Warp Matrix", &show_mat_warp_matrix);
            ImGui::Checkbox("Show Text Edit Window", &show_text_editor_window);
            ImGui::Checkbox("Show Tab Window", &show_tab_window);
            ImGui::Checkbox("Show Node Editor Window", &show_node_editor_window);
            ImGui::Checkbox("Show Curve Demo Window", &show_curve_demo_window);
            ImGui::Checkbox("Show Spline Demo Window", &show_spline_demo_window);
            ImGui::Checkbox("ZmoQuat Demo Window", &show_zmoquat_window);
            ImGui::Checkbox("Zmo Demo Window", &show_zmo_window);
            ImGui::Checkbox("Toggle Demo Window", &show_toggle_window);
            ImGui::Checkbox("TexInspect Window", &show_tex_inspect_window);

            // show hotkey window
            if (ImGui::Button("Edit Hotkeys"))
            {
                ImGui::OpenPopup("HotKeys Editor");
            }

            // Handle hotkey popup
            ImHotKey::Edit(hotkeys.data(), hotkeys.size(), "HotKeys Editor");
            int hotkey = ImHotKey::GetHotKey(hotkeys.data(), hotkeys.size());
            if (hotkey != -1)
            {
                // handle the hotkey index!
            }

            ImVec2 displayedTextureSize(256,256);
            ImGui::Image((ImTextureID)(uint64_t)ImageTexture, displayedTextureSize);
            {
                ImRect rc = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                ImVec2 mouseUVCoord = (io.MousePos - rc.Min) / rc.GetSize();
                if (ImGui::IsItemHovered() && mouseUVCoord.x >= 0.f && mouseUVCoord.y >= 0.f)
                {
                    ImGui::ImageInspect(image.w, image.h, 
                                        (const unsigned char*)image.data, mouseUVCoord, 
                                        displayedTextureSize);
                }
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Frames since last input: %d", ImGui::GetIO().FrameCountSinceLastInput);
            ImGui::Text("Time Date: %s", ImGuiHelper::date_time_string().c_str());
            ImGui::Text("User Name: %s", ImGuiHelper::username().c_str());
            ImGui::Text("Home path: %s", ImGuiHelper::home_path().c_str());
            ImGui::Text("Temp path: %s", ImGuiHelper::temp_path().c_str());
            ImGui::Text("Working path: %s", ImGuiHelper::cwd_path().c_str());
            ImGui::Text("Exec path: %s", ImGuiHelper::exec_path().c_str());
            ImGui::Text("Setting path: %s", ImGuiHelper::settings_path("ImGui Example").c_str());
            ImGui::Text("Memory usage: %zu", ImGuiHelper::memory_usage());
            ImGui::Text("Memory Max usage: %zu", ImGuiHelper::memory_max_usage());
            ImGui::End();
        }

        // Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);         // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Show FileDialog demo window
        if (show_file_dialog_window)
        {
            show_file_dialog_demo_window(&filedialog, &show_file_dialog_window);
        }

        // Show Portable File Dialogs, open native file dialog and native notification not working for emscripten
        if (show_portable_file_dialogs)
        {
            ImGui::SetNextWindowSize(ImVec2(640, 300), ImGuiCond_FirstUseEver);
            ImGui::Begin("Portable FileDialog window",&show_portable_file_dialogs, ImGuiWindowFlags_NoScrollbar);

            // show Message
            static int message_type = 0;
            static int message_icon = 0;
            if (ImGui::Button("Message"))
            {
                auto m = pfd::message("Personal Message", "You are an amazing person, don't let anyone make you think otherwise.",
                                        (pfd::choice)message_type,
                                        (pfd::icon)message_icon);

                // Optional: do something while waiting for user action
                for (int i = 0; i < 10 && !m.ready(1000); ++i);
                //    std::cout << "Waited 1 second for user input...\n";

                // Do something according to the selected button
                if (m.ready())
                {
                    switch (m.result())
                    {
                        case pfd::button::yes: std::cout << "User agreed.\n"; break;
                        case pfd::button::no: std::cout << "User disagreed.\n"; break;
                        case pfd::button::cancel: std::cout << "User freaked out.\n"; break;
                        default: break; // Should not happen
                    }
                }
                else
                    m.kill();
            }
            ImGui::SameLine(); ImGui::RadioButton("Ok", &message_type, 0);
            ImGui::SameLine(); ImGui::RadioButton("Ok_Cancel", &message_type, 1);
            ImGui::SameLine(); ImGui::RadioButton("Yes_no", &message_type, 2);
            ImGui::SameLine(); ImGui::RadioButton("Yes_no_cancel", &message_type, 3);
            ImGui::SameLine(); ImGui::RadioButton("Abort_retry_ignore", &message_type, 4);
            ImGui::Indent(64);
            ImGui::RadioButton("Info##icon", &message_icon, 0); ImGui::SameLine();
            ImGui::RadioButton("Warning##icon", &message_icon, 1); ImGui::SameLine();
            ImGui::RadioButton("Error##icon", &message_icon, 2); ImGui::SameLine();
            ImGui::RadioButton("Question##icon", &message_icon, 3);

            ImGui::End();
        }

        // Show Memory Edit window
        if (mem_edit.Open)
        {
            mem_edit.DrawWindow("Memory Editor", data, 0x1000, 0, &mem_edit.Open, 768);
        }

        // Show Markdown Window
        if (show_markdown_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
            ImGui::Begin("Markdown window",&show_markdown_window, ImGuiWindowFlags_NoScrollbar);
            std::string help_doc =          get_file_contents();
            mdConfig.linkCallback =         LinkCallback;
            mdConfig.tooltipCallback =      NULL;
            mdConfig.imageCallback =        ImageCallback;
            mdConfig.linkIcon =             ICON_FA_LINK;
            mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
            mdConfig.headingFormats[1] =    { io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : nullptr, true };
            mdConfig.headingFormats[2] =    { io.Fonts->Fonts.size() > 2 ? io.Fonts->Fonts[2] : nullptr, false };
            mdConfig.userData =             NULL;
            mdConfig.formatCallback =       ExampleMarkdownFormatCallback;
            ImGui::Markdown( help_doc.c_str(), help_doc.length(), mdConfig );
            ImGui::End();
        }

        // Show Extra widget Window
        if (show_widget_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
            ImGui::Begin("Extra Widget", &show_widget_window);
            ImGui::ShowExtraWidgetDemoWindow();
            ImGui::End();
        }

        // Show Kalman Window
        if (show_kalman_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
            ImGui::Begin("Kalman Demo", &show_kalman_window);
            ImGui::ShowImKalmanDemoWindow();
            ImGui::End();
        }

        // Show FFT Window
        if (show_fft_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1024, 1024), ImGuiCond_FirstUseEver);
            ImGui::Begin("FFT Demo", &show_fft_window);
            ImGui::ShowImFFTDemoWindow();
            ImGui::End();
        }

         // Show STFT Window
        if (show_stft_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1024, 1024), ImGuiCond_FirstUseEver);
            ImGui::Begin("STFT Demo", &show_stft_window);
            ImGui::ShowImSTFTDemoWindow();
            ImGui::End();
        }

        // Show ImMat line demo
        if (show_mat_draw_window)
        {
            ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
            ImGui::Begin("ImMat draw Demo", &show_mat_draw_window);
            DrawLineDemo();
            ImGui::End();
        }

        // Show ImMat warp matrix demo
        if (show_mat_warp_matrix)
        {
            ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
            ImGui::Begin("ImMat warp matrix Demo", &show_mat_warp_matrix);
            WarpMatrixDemo();
            ImGui::End();
        }

        // Show Text Edit Window
        if (show_text_editor_window)
        {
            editor.text_edit_demo(&show_text_editor_window);
        }

        // Show Tab Window
        if (show_tab_window)
        {
            ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: TabWindow", &show_tab_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowAddonsTabWindow();   // see its code for further info         
            }
            ImGui::End();
        }

        // Show Node Editor Window
        if (show_node_editor_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1024,1024), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: Node Editor", &show_node_editor_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowNodeEditorWindow();   // see its code for further info         
            }
            ImGui::End();
        }

        // Show Curve Demo Window
        if (show_curve_demo_window)
        {
            ImGui::SetNextWindowSize(ImVec2(800,600), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: Curve Demo", &show_curve_demo_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowCurveDemo();   // see its code for further info         
            }
            ImGui::End();
        }

        // Show Spline Demo Window
        if (show_spline_demo_window)
        {
            ImGui::SetNextWindowSize(ImVec2(800,800), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: Spline Demo", &show_spline_demo_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowSplineDemo();   // see its code for further info         
            }
            ImGui::End();
        }

        // Show Zmo Quat Window
        if (show_zmoquat_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1280, 900), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("ZMOQuat", &show_zmoquat_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowQuatDemo();
            }
            ImGui::End();
        }

        // Show Zmo Window
        if (show_zmo_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
            ImGui::Begin("##ZMO", &show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
            ImGuizmo::ShowImGuiZmoDemo();
            ImGui::End();
        }

        // Show Toggle Window
        if (show_toggle_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
            ImGui::Begin("##Toggle", &show_toggle_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
            ImGui::imgui_toggle_example();
            ImGui::End();
        }

        // Show TexInspect Window
        if (show_tex_inspect_window)
        {
            ImGuiTexInspect::ShowImGuiTexInspectDemo(&show_tex_inspect_window);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    EMSCRIPTEN_MAINLOOP_END;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}