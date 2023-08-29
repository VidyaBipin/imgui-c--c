#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_helper.h>
#include <application.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <imgui_markdown.h>
#include <imgui_memory_editor.h>
#include <implot.h>
#include <ImGuiFileDialog.h>
#include <imgui_extra_widget.h>
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
#include <ImCoolbar.h>
#include <portable-file-dialogs.h>

#if IMGUI_VULKAN_SHADER
#include <ImVulkanShader.h>
#include <imvk_mat_shader.h>
//#define TEST_VKIMAGEMAT
#endif
#include <immat.h>
#include "Config.h"

// Init HotKey
static std::vector<ImHotKey::HotKey> hotkeys = 
{ 
    {"Layout", "Reorder nodes in a simpler layout", 0xFFFF26E0},
    {"Save", "Save the current graph", 0xFFFF1FE0},
    {"Load", "Load an existing graph file", 0xFFFF18E0},
    {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
    {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
};

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

static void Show_Coolbar_demo_window()
{
    auto coolbar_button     = [](const char* label) -> bool
    {
		float w         = ImGui::GetCoolBarItemWidth();
        ImGui::SetWindowFontScale(ImGui::GetCoolBarItemScale());
		bool res = ImGui::Button(label, ImVec2(w, w));
        ImGui::SetWindowFontScale(1.0);
		return res;
	};
    ImGui::ImCoolBarConfig config;
    auto viewport = ImGui::GetWindowViewport();
    config.anchor = ImVec2(0.5, 1.0);
    ImGui::SetNextWindowViewport(viewport->ID);
    if (ImGui::BeginCoolBar("##CoolBarHorizontal", ImCoolBarFlags_Horizontal, config))
    {
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("A")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("B")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("C")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("D")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("E")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("F")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("G")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("H")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("I")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("J")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("K")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("L")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("M")) { }
		}
		ImGui::EndCoolBar();
	}
    config.anchor = ImVec2(1.0, 0.5);
    ImGui::SetNextWindowViewport(viewport->ID);
    if (ImGui::BeginCoolBar("##CoolBarVertical", ImCoolBarFlags_Vertical, config))
    {
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("a")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("b")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("c")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("d")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("e")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("f")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("g")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("h")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("i")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("j")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("k")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("l")) { }
		}
		if (ImGui::CoolBarItem()) {
			if (coolbar_button("m")) { }
		}
		ImGui::EndCoolBar();
	}
}

class Example
{
public:
    Example() 
    {
        // load file dialog resource
#ifdef DEFAULT_CONFIG_PATH
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
#else
        std::string bookmark_path = "bookmark.ini";
#endif

        prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

        // init memory edit
        mem_edit.Open = false;
        mem_edit.OptShowDataPreview = true;
        mem_edit.OptAddrDigitsCount = 8;
        data = malloc(0x400);
        // init color inspact
        color_bar(image, 0, 0, 255, 191);
        gray_bar(image, 0, 192, 255, 255, 13);
        // init draw mat
        draw_mat.clean(ImPixel(0.f, 0.f, 0.f, 1.f));
#if IMGUI_VULKAN_SHADER
        ImGui::ImMat test_image(256, 256, 4, 1u, 4);
        for (int y = 0; y < 256; y++)
        {
            for (int x = 0; x < 256; x++)
            {
                float dx = x + .5f;
                float dy = y + .5f;
                float dv = sinf(x * 0.02f) + sinf(0.03f * (x + y)) + sinf(sqrtf(0.4f * (dx * dx + dy * dy) + 1.f));
                test_image.at<unsigned char>(x, y, 3) = UCHAR_MAX;
                test_image.at<unsigned char>(x, y, 2) = fabsf(sinf(dv * 3.141592f + 4.f * 3.141592f / 3.f)) * UCHAR_MAX;
                test_image.at<unsigned char>(x, y, 1) = fabsf(sinf(dv * 3.141592f + 2.f * 3.141592f / 3.f)) * UCHAR_MAX;
                test_image.at<unsigned char>(x, y, 0) = fabsf(sinf(dv * 3.141592f)) * UCHAR_MAX;
            }
        }

        vkdev = ImGui::get_gpu_device(ImGui::get_default_gpu_index());
        ImGui::VkTransfer tran(vkdev);
#ifdef TEST_VKIMAGEMAT
        opt_vkimage.use_image_storage = true;
        opt_vkimage.blob_vkallocator = vkdev->acquire_blob_allocator();
        opt_vkimage.staging_vkallocator = vkdev->acquire_staging_allocator();
        tran.record_upload(test_image, test_vkImageMat, opt_vkimage, false);
        tran.submit_and_wait();
#else
        opt.blob_vkallocator = vkdev->acquire_blob_allocator();
        opt.staging_vkallocator = vkdev->acquire_staging_allocator();
        tran.record_upload(test_image, test_vkMat, opt, false);
        tran.submit_and_wait();
#endif

        std::vector<ImGui::vk_specialization_type> specializations(0);
        std::vector<uint32_t> spirv_data;
#ifdef TEST_VKIMAGEMAT
        if (ImGui::compile_spirv_module(glsl_copy, opt_vkimage, spirv_data) == 0)
        {
            vkimage_copy = new ImGui::Pipeline(vkdev);
            vkimage_copy->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        }
#else
        if (ImGui::compile_spirv_module(glsl_copy, opt, spirv_data) == 0)
        {
            vk_copy = new ImGui::Pipeline(vkdev);
            vk_copy->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        }
#endif
#endif
    };
    ~Example() 
    {
        if (data)
            free(data); 

        // Store file dialog bookmark
#ifdef DEFAULT_CONFIG_PATH
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
#else
        std::string bookmark_path = "bookmark.ini";
#endif
        end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
        if (ImageTexture) { ImGui::ImDestroyTexture(ImageTexture); ImageTexture = 0; }
        if (DrawMatTexture) { ImGui::ImDestroyTexture(DrawMatTexture); DrawMatTexture = 0; }
#if IMGUI_VULKAN_SHADER
        if (vkdev)
        {
#ifdef TEST_VKIMAGEMAT
            test_vkImageMat.release();
            if (opt_vkimage.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt_vkimage.blob_vkallocator); }
            if (opt_vkimage.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt_vkimage.staging_vkallocator); }
            if (vkimage_copy) delete vkimage_copy;
#else
            test_vkMat.release();
            if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); }
            if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); }
            if (vk_copy) delete vk_copy;
#endif
        }
#endif
    }

public:
    // init file dialog
    ImGuiFileDialog filedialog;

    // init memory edit
    MemoryEditor mem_edit;
    void* data = nullptr;

    // Init MarkDown
    ImGui::MarkdownConfig mdConfig;

    // Init Colorful Text Edit
    TextEditor editor;

public:
    bool show_demo_window = true;
    bool show_viewport_fullscreen = false;
    bool show_another_window = false;
    bool show_implot_window = false;
    bool show_file_dialog_window = false;
    bool show_markdown_window = false;
    bool show_widget_window = false;
    bool show_mat_draw_window = false;
    bool show_mat_warp_matrix = false;
    bool show_kalman_window = false;
    bool show_fft_window = false;
    bool show_stft_window = false;
    bool show_text_editor_window = false;
    bool show_tab_window = false;
    bool show_node_editor_window = false;
    bool show_curve_demo_window = false;
    bool show_spline_demo_window = false;
    bool show_zmoquat_window = false;
    bool show_zmo_window = false;
    bool show_toggle_window = false;
    bool show_tex_inspect_window = false;
    bool show_portable_file_dialogs = false;
    bool show_coolbar_window = false;
public:
    void DrawLineDemo();
    void WarpMatrixDemo();
    void ImVulkanTestWindow(const char* name, bool* p_open, ImGuiWindowFlags flags);
    std::string get_file_contents(const char *filename);
    static ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ );

#if IMGUI_VULKAN_SHADER
public:
    bool show_shader_window = false;
#ifdef TEST_VKIMAGEMAT
    ImGui::VkImageMat test_vkImageMat;
    ImGui::Option opt_vkimage;
    ImGui::Pipeline * vkimage_copy = nullptr;
#else
    ImGui::VkMat test_vkMat;
    ImGui::Option opt;
    ImGui::Pipeline * vk_copy = nullptr;
#endif
    ImGui::VulkanDevice* vkdev = nullptr;
#endif
public:
    ImGui::ImMat image {ImGui::ImMat(256, 256, 4, 1u, 4)};
    ImGui::ImMat draw_mat {ImGui::ImMat(512, 512, 4, 1u, 4)};
    ImGui::ImMat small_mat {ImGui::ImMat(128, 128, 4, 4u, 4)};
    ImTextureID ImageTexture = 0;
    ImTextureID DrawMatTexture = 0;
};

std::string Example::get_file_contents(const char *filename)
{
#ifdef DEFAULT_DOCUMENT_PATH
    std::string file_path = std::string(DEFAULT_DOCUMENT_PATH) + std::string(filename);
#else
    std::string file_path = std::string(filename);
#endif
    std::ifstream infile(file_path, std::ios::in | std::ios::binary);
    if (infile.is_open())
    {
        std::ostringstream contents;
        contents << infile.rdbuf();
        infile.close();
        return(contents.str());
    }
    else
    {
        std::string test = 
            "Syntax Tests For imgui_markdown\n"
            "Test - Headers\n"
            "# Header 1\n"
            "Paragraph\n"
            "## Header 2\n"
            "Paragraph\n"
            "### Header 3\n"
            "Paragraph\n"
            "Test - Emphasis\n"
            "*Emphasis with stars*\n"
            "_Emphasis with underscores_\n"
            "**Strong emphasis with stars**\n"
            "__Strong emphasis with underscores__\n"
            "_*_\n"
            "**_**\n"
            "Test - Emphasis In List\n"
            "  * *List emphasis with stars*\n"
            "    * *Sublist with emphasis*\n"
            "    * Sublist without emphasis\n"
            "    * **Sublist** with *some* emphasis\n"
            "  * _List emphasis with underscores_\n"
            "Test - Emphasis In Indented Paragraph\n"
            "  *Indented emphasis with stars*\n"
            "    *Double indent with emphasis*\n"
            "    Double indent without emphasis\n"
            "    **Double indent** with *some* emphasis\n"
            "  _Indented emphasis with underscores_\n"
            ;
        return test;
    }
}

ImGui::MarkdownImageData Example::ImageCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    char image_url[MAX_PATH_BUFFER_SIZE] = {0};
    strncpy(image_url, data_.link, data_.linkLength);
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

void Example::LinkCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    std::string url( data_.link, data_.linkLength );
    std::string command = "open " + url;
    if( !data_.isImage )
    {
        system(command.c_str());
    }
}

void Example::ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
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

void Example::DrawLineDemo()
{
    float t = (float)ImGui::GetTime();
    float h = abs(sin(t * 0.2));
    float s = abs(sin(t * 0.1)) * 0.5 + 0.4;
    float h2 = abs(sin(t * 0.4));
    float h3 = abs(sin(t * 0.8));
    static int offset_x = 0;
    static int offset_y = 0;
    static int step_x = 2;
    static int step_y = 3;
    ImVec4 base_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImVec4 light_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImVec4 t_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.5f, base_color.x, base_color.y, base_color.z);
    ImGui::ColorConvertHSVtoRGB(h2, s, 0.5f, light_color.x, light_color.y, light_color.z);
    ImGui::ColorConvertHSVtoRGB(h3, s, 0.5f, t_color.x, t_color.y, t_color.z);
    static float arc = 0.0;
    draw_mat.clean(ImPixel(0.f, 0.f, 0.f, 1.f));
    arc += 2 * M_PI / 64 / 32;
    if (arc > 2 * M_PI / 64) arc = 0;
    float cx = draw_mat.w * 0.5f, cy = draw_mat.h * 0.5f;
    ImPixel line_color(base_color.x, base_color.y, base_color.z, 1.f);
    ImPixel circle_color(light_color.x, light_color.y, light_color.z, 1.f);
    ImPixel text_color(t_color.x, t_color.y, t_color.z, 1.f);
    small_mat.clean(text_color);
    small_mat.draw_line(ImPoint(0, 0), ImPoint(small_mat.w - 1, 0), 1, ImPixel(0, 0, 0, 1));
    small_mat.draw_line(ImPoint(0, 0), ImPoint(0, small_mat.h - 1), 1, ImPixel(0, 0, 0, 1));
    small_mat.draw_line(ImPoint(small_mat.w - 1, 0), ImPoint(small_mat.w - 1, small_mat.h - 1), 1, ImPixel(0, 0, 0, 1));
    small_mat.draw_line(ImPoint(0, small_mat.h - 1), ImPoint(small_mat.w - 1, small_mat.h - 1), 1, ImPixel(0, 0, 0, 1));
    small_mat.draw_line(ImPoint(small_mat.w / 2, 0), ImPoint(small_mat.w / 2, small_mat.h - 1), 1, ImPixel(0, 0, 0, 1));
    small_mat.draw_line(ImPoint(0, small_mat.h / 2), ImPoint(small_mat.w - 1, small_mat.h / 2), 1, ImPixel(0, 0, 0, 1));
#if IMGUI_VULKAN_SHADER
    ImGui::VkMat small_vkmat(small_mat);
#endif

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

    std::string text_str = "字体测试\nFont Test\n字体Test\nFont测试";
    //auto str_mat = ImGui::CreateTextMat(text_str.c_str(), text_color, 1.0);
    //ImGui::ImageMatCopyTo(str_mat, draw_mat, ImVec2(draw_mat.w / 2 - str_mat.w / 2, draw_mat.h / 2 - str_mat.h / 2));
    ImGui::DrawTextToMat(draw_mat, ImPoint(50, 50), text_str.c_str(), text_color, 1.0);

    ImGui::ImMatToTexture(draw_mat, DrawMatTexture);

#if IMGUI_VULKAN_SHADER
    ImGui::ImCopyToTexture(DrawMatTexture, (unsigned char*)&small_vkmat, small_vkmat.w, small_vkmat.h, small_vkmat.c, offset_x, offset_y, true);
#else
    ImGui::ImCopyToTexture(DrawMatTexture, (unsigned char*)&small_mat, small_mat.w, small_mat.h, small_mat.c, offset_x, offset_y, true);
#endif

    offset_x += step_x;
    offset_y += step_y;
    if (offset_x < 0 || offset_x + small_mat.w >= draw_mat.w) { step_x = -step_x; offset_x += step_x; }
    if (offset_y < 0 || offset_y + small_mat.h >= draw_mat.h) { step_y = -step_y; offset_y += step_y; }
    
    ImGui::Image(DrawMatTexture, ImVec2(draw_mat.w, draw_mat.h));
}

void Example::WarpMatrixDemo()
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

#if IMGUI_VULKAN_SHADER
void Example::ImVulkanTestWindow(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGui::Begin(name, p_open, flags);
    static int loop_count = 200;
    static int block_count = 20;
    static int cmd_count = 1;
    static float fp32[8] = {0.f};
    static float fp32v4[8] = {0.f};
    static float fp32v8[8] = {0.f};
    static float fp16pv4[8] = {0.f};
    static float fp16pv8[8] = {0.f};
    static float fp16s[8] = {0.f};
    static float fp16sv4[8] = {0.f};
    static float fp16sv8[8] = {0.f};
    int device_count = ImGui::get_gpu_count();
    auto print_result = [](float gflops)
    {
        std::string result;
        if (gflops == -1)
            result = "  error";
        if (gflops == -233)
            result = "  not supported";
        if (gflops == 0)
            result = "  not tested";
        if (gflops > 1000)
            result = "  " + std::to_string(gflops / 1000.0) + " TFLOPS";
        else
            result = "  " + std::to_string(gflops) + " GFLOPS";
        return result;
    };
    for (int i = 0; i < device_count; i++)
    {
        ImGui::VulkanDevice* vkdev = ImGui::get_gpu_device(i);
        uint32_t driver_version = vkdev->info.driver_version();
        uint32_t api_version = vkdev->info.api_version();
        int device_type = vkdev->info.type();
        std::string driver_ver = std::to_string(VK_VERSION_MAJOR(driver_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(driver_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(driver_version));
        std::string api_ver =   std::to_string(VK_VERSION_MAJOR(api_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(api_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(api_version));
        std::string device_name = vkdev->info.device_name();
        uint32_t gpu_memory_budget = vkdev->get_heap_budget();
        uint32_t gpu_memory_usage = vkdev->get_heap_usage();
        ImGui::Text("Device[%d]", i);
        ImGui::Text("Driver:%s", driver_ver.c_str());
        ImGui::Text("   API:%s", api_ver.c_str());
        ImGui::Text("  Name:%s", device_name.c_str());
        ImGui::Text("Memory:%uMB/%uMB", gpu_memory_usage, gpu_memory_budget);
        ImGui::Text("Device Type:%s", device_type == 0 ? "Discrete" : device_type == 1 ? "Integrated" : device_type == 2 ? "Virtual" : "CPU");
        std::string buffon_label = "Perf Test##" + std::to_string(i);
        if (ImGui::Button(buffon_label.c_str(), ImVec2(120, 20)))
        {
            int _loop_count = device_type == 0 ? loop_count : loop_count / 5;
            fp32[i]     = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 0);
            fp32v4[i]   = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 1);
            fp32v8[i]   = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 2);
            fp16pv4[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 1, 1, 1);
            fp16pv8[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 1, 1, 2);
            fp16s[i]    = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 0);
            fp16sv4[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 1);
            fp16sv8[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 2);
        }
        ImGui::Text(" FP32 Scalar :%s", print_result(fp32[i]).c_str());
        ImGui::Text("   FP32 Vec4 :%s", print_result(fp32v4[i]).c_str());
        ImGui::Text("   FP32 Vec8 :%s", print_result(fp32v8[i]).c_str());
        ImGui::Text("  FP16p Vec4 :%s", print_result(fp16pv4[i]).c_str());
        ImGui::Text("  FP16p Vec8 :%s", print_result(fp16pv8[i]).c_str());
        ImGui::Text("FP16s Scalar :%s", print_result(fp16s[i]).c_str());
        ImGui::Text("  FP16s Vec4 :%s", print_result(fp16sv4[i]).c_str());
        ImGui::Text("  FP16s Vec8 :%s", print_result(fp16sv8[i]).c_str());
        
        ImGui::Separator();
    }

    static double avg_copy_time = 0;
#ifdef TEST_VKIMAGEMAT
    ImGui::VkImageMat image_result;
    image_result.create_like(test_vkImageMat, opt_vkimage.blob_vkallocator);
    if (ImGui::Button("Test Copy", ImVec2(120, 20)))
    {
        double time = 0;
        ImGui::VkCompute cmd_image(vkdev, "VkImageCopy");
        std::vector<ImGui::VkImageMat> bindings_image(2);
        bindings_image[0] = test_vkImageMat;
        bindings_image[1] = image_result;
        std::vector<ImGui::vk_constant_type> constants_image(4);
        constants_image[0].i = test_vkImageMat.w;
        constants_image[1].i = test_vkImageMat.h;
        constants_image[2].i = test_vkImageMat.c;
        constants_image[3].i = test_vkImageMat.cstep;
        for (int i =0; i < 100; i++)
        {
            double t0 = GetSysCurrentTime();
            cmd_image.record_pipeline(vkimage_copy, bindings_image, constants_image, image_result);
            cmd_image.submit_and_wait();
            time += GetSysCurrentTime() - t0;
        }
        avg_copy_time = time / 100;
    }
#else
    ImGui::VkMat result;
    result.create_like(test_vkMat, opt.blob_vkallocator);
    if (ImGui::Button("Test Copy", ImVec2(120, 20)))
    {
        double time = 0;
        ImGui::VkCompute cmd(vkdev, "VkMatCopy");
        std::vector<ImGui::VkMat> bindings(2);
        bindings[0] = test_vkMat;
        bindings[1] = result;
        std::vector<ImGui::vk_constant_type> constants(4);
        constants[0].i = test_vkMat.w;
        constants[1].i = test_vkMat.h;
        constants[2].i = test_vkMat.c;
        constants[3].i = test_vkMat.cstep;
        for (int i =0; i < 100; i++)
        {
            double t0 = GetSysCurrentTime();
            cmd.record_pipeline(vk_copy, bindings, constants, result);
            cmd.submit_and_wait();
            time += GetSysCurrentTime() - t0;
            cmd.reset();
        }
        avg_copy_time = time / 100;
    }
    ImGui::Text("VkMat copy avg:%fs", avg_copy_time);
    if (!result.empty())
    {
        ImTextureID vk_texture = nullptr;
        ImGui::ImMatToTexture(result, vk_texture);
        ImGui::Image(vk_texture, ImVec2(result.w, result.h));
    }
#endif
    ImGui::End();
}
#endif

void Example_Initialize(void** handle)
{
    srand((unsigned int)time(0));
    *handle = new Example();
    Example * example = (Example *)*handle;
    ImPlot::CreateContext();
}

void Example_Finalize(void** handle)
{
    if (handle && *handle)
    {
        Example * example = (Example *)*handle;
        delete example;
        *handle = nullptr;
    }
    ImPlot::DestroyContext();
}

bool Example_Frame(void* handle, bool app_will_quit)
{
    bool app_done = false;
    auto& io = ImGui::GetIO();
    Example * example = (Example *)handle;
    if (!example)
        return true;
    if (!example->ImageTexture) 
    {
        example->ImageTexture = ImGui::ImCreateTexture(example->image.data, example->image.w, example->image.h);
    }
    // Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (example->show_demo_window)
        ImGui::ShowDemoWindow(&example->show_demo_window);

    // Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &example->show_demo_window);      // Edit bools storing our window open/close state
        if (ImGui::Checkbox("Full Screen Window", &example->show_viewport_fullscreen))
        {
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                auto platformio = ImGui::GetPlatformIO();
                //if (platformio.Platform_FullScreen) platformio.Platform_FullScreen(ImGui::GetMainViewport(), example->show_viewport_fullscreen);
                if (platformio.Platform_FullScreen) platformio.Platform_FullScreen(ImGui::GetWindowViewport(), example->show_viewport_fullscreen);
            }
            else
                Application_FullScreen(example->show_viewport_fullscreen);
        }
        ImGui::Checkbox("Another Window", &example->show_another_window);
        ImGui::Checkbox("ImPlot Window", &example->show_implot_window);
        ImGui::Checkbox("File Dialog Window", &example->show_file_dialog_window);
        ImGui::Checkbox("Portable File Dialogs", &example->show_portable_file_dialogs);
        ImGui::Checkbox("Memory Edit Window", &example->mem_edit.Open);
        ImGui::Checkbox("Markdown Window", &example->show_markdown_window);
        ImGui::Checkbox("Extra Widget Window", &example->show_widget_window);
        ImGui::Checkbox("Kalman Window", &example->show_kalman_window);
        ImGui::Checkbox("FFT Window", &example->show_fft_window);
        ImGui::Checkbox("STFT Window", &example->show_stft_window);
        ImGui::Checkbox("ImMat Draw Window", &example->show_mat_draw_window);
        ImGui::Checkbox("ImMat Warp Matrix", &example->show_mat_warp_matrix);
        ImGui::Checkbox("Text Edit Window", &example->show_text_editor_window);
        ImGui::Checkbox("Tab Window", &example->show_tab_window);
        ImGui::Checkbox("Node Editor Window", &example->show_node_editor_window);
        ImGui::Checkbox("Curve Demo Window", &example->show_curve_demo_window);
        ImGui::Checkbox("Spline Demo Window", &example->show_spline_demo_window);
        ImGui::Checkbox("ZmoQuat Demo Window", &example->show_zmoquat_window);
        ImGui::Checkbox("Zmo Demo Window", &example->show_zmo_window);
        ImGui::Checkbox("Toggle Demo Window", &example->show_toggle_window);
        ImGui::Checkbox("TexInspect Window", &example->show_tex_inspect_window);
        ImGui::Checkbox("Coolbar Window", &example->show_coolbar_window);

#if IMGUI_VULKAN_SHADER
        ImGui::Checkbox("Show Vulkan Shader Test Window", &example->show_shader_window);
#endif
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
        ImGui::Image((ImTextureID)(uint64_t)example->ImageTexture, displayedTextureSize);
        {
            ImRect rc = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            ImVec2 mouseUVCoord = (io.MousePos - rc.Min) / rc.GetSize();
            if (ImGui::IsItemHovered() && mouseUVCoord.x >= 0.f && mouseUVCoord.y >= 0.f)
            {
                ImGui::ImageInspect(example->image.w, example->image.h, 
                                    (const unsigned char*)example->image.data, mouseUVCoord, 
                                    displayedTextureSize);
            }
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ImGui::GetIO().DeltaTime * 1000.f, ImGui::GetIO().Framerate);
        ImGui::Text("Frames since last input: %d", ImGui::GetIO().FrameCountSinceLastInput);
        ImGui::Text("Time Date: %s", ImGuiHelper::date_time_string().c_str());
        ImGui::Separator();
        ImGui::Text("User Name: %s", ImGuiHelper::username().c_str());
        ImGui::Text("Home path: %s", ImGuiHelper::home_path().c_str());
        ImGui::Text("Temp path: %s", ImGuiHelper::temp_path().c_str());
        ImGui::Text("Working path: %s", ImGuiHelper::cwd_path().c_str());
        ImGui::Text("Exec path: %s", ImGuiHelper::exec_path().c_str());
        ImGui::Text("Setting path: %s", ImGuiHelper::settings_path("ImGui Example").c_str());
        ImGui::Separator();
        ImGui::Text("DataHome path: %s", ImGuiHelper::getDataHome().c_str());
        ImGui::Text("Config path: %s", ImGuiHelper::getConfigHome().c_str());
        ImGui::Text("Cache path: %s", ImGuiHelper::getCacheDir().c_str());
        ImGui::Text("State path: %s", ImGuiHelper::getStateDir().c_str());
        ImGui::Text("Desktop path: %s", ImGuiHelper::getDesktopFolder().c_str());
        ImGui::Text("Documents path: %s", ImGuiHelper::getDocumentsFolder().c_str());
        ImGui::Text("Download path: %s", ImGuiHelper::getDownloadFolder().c_str());
        ImGui::Text("Pictures path: %s", ImGuiHelper::getPicturesFolder().c_str());
        ImGui::Text("Public path: %s", ImGuiHelper::getPublicFolder().c_str());
        ImGui::Text("Music path: %s", ImGuiHelper::getMusicFolder().c_str());
        ImGui::Text("Video path: %s", ImGuiHelper::getVideoFolder().c_str());
        ImGui::Separator();
        ImGui::Text("Memory usage: %zu", ImGuiHelper::memory_usage());
        ImGui::Text("Memory Max usage: %zu", ImGuiHelper::memory_max_usage());
        ImGui::End();
    }

    // Show another simple window.
    if (example->show_another_window)
    {
        ImGui::Begin("Another Window", &example->show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            example->show_another_window = false;
        ImGui::End();
    }

    // Show ImPlot simple window
    if (example->show_implot_window)
    {
        ImPlot::ShowDemoWindow(&example->show_implot_window);
    }

    // Show FileDialog demo window
    if (example->show_file_dialog_window)
    {
        show_file_dialog_demo_window(&example->filedialog, &example->show_file_dialog_window);
    }

    // Show Portable File Dialogs
    if (example->show_portable_file_dialogs)
    {
        ImGui::SetNextWindowSize(ImVec2(640, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Portable FileDialog window",&example->show_portable_file_dialogs, ImGuiWindowFlags_NoScrollbar);
        // select folder
        if (ImGui::Button("Select Folder"))
        {
            auto dir = pfd::select_folder("Select any directory", pfd::path::home()).result();
            // std::cout << "Selected dir: " << dir << "\n";
        }

        // open file
        if (ImGui::Button("Open File"))
        {
            auto f = pfd::open_file("Choose files to read", pfd::path::home(),
                                { "Text Files (.txt .text)", "*.txt *.text",
                                    "All Files", "*" },
                                    pfd::opt::multiselect);
            // for (auto const &name : f.result()) std::cout << " " + name; std::cout << "\n";
        }

        // save file
        if (ImGui::Button("Save File"))
        {
            auto f = pfd::save_file("Choose file to save",
                                    pfd::path::home() + pfd::path::separator() + "readme.txt",
                                    { "Text Files (.txt .text)", "*.txt *.text" },
                                    pfd::opt::force_overwrite);
            // std::cout << "Selected file: " << f.result() << "\n";
        }

        // show Notification
        static int notify_type = 0;
        if (ImGui::Button("Notification"))
        {
            switch (notify_type)
            {
                case 0: pfd::notify("Info Notification", "Notification from imgui example!", pfd::icon::info); break;
                case 1: pfd::notify("Warning Notification", "Notification from imgui example!", pfd::icon::warning); break;
                case 2: pfd::notify("Error Notification", "Notification from imgui example!", pfd::icon::error); break;
                case 3: pfd::notify("Question Notification", "Notification from imgui example!", pfd::icon::question); break;
                default: break;
            }
        }
        ImGui::SameLine(); ImGui::RadioButton("Info", &notify_type, 0);
        ImGui::SameLine(); ImGui::RadioButton("Warning", &notify_type, 1);
        ImGui::SameLine(); ImGui::RadioButton("Error", &notify_type, 2);
        ImGui::SameLine(); ImGui::RadioButton("Question", &notify_type, 3);

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
    if (example->mem_edit.Open)
    {
        static int i = 0;
        int * test_point = (int *)example->data;
        *test_point = i; i++;
        example->mem_edit.DrawWindow("Memory Editor", example->data, 0x400, 0, &example->mem_edit.Open, 768);
    }

    // Show Markdown Window
    if (example->show_markdown_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("Markdown window",&example->show_markdown_window, ImGuiWindowFlags_NoScrollbar);
        std::string help_doc =                   example->get_file_contents("README.md");
        example->mdConfig.linkCallback =         example->LinkCallback;
        example->mdConfig.tooltipCallback =      nullptr;
        example->mdConfig.imageCallback =        example->ImageCallback;
        example->mdConfig.linkIcon =             "->";
        example->mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
        example->mdConfig.headingFormats[1] =    { io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : nullptr, true };
        example->mdConfig.headingFormats[2] =    { io.Fonts->Fonts.size() > 2 ? io.Fonts->Fonts[2] : nullptr, false };
        example->mdConfig.userData =             nullptr;
        example->mdConfig.formatCallback =       example->ExampleMarkdownFormatCallback;
        ImGui::Markdown( help_doc.c_str(), help_doc.length(), example->mdConfig );
        ImGui::End();
    }

    // Show Extra widget Window
    if (example->show_widget_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("Extra Widget", &example->show_widget_window);
        ImGui::ShowExtraWidgetDemoWindow();
        ImGui::End();
    }

    // Show Kalman Window
    if (example->show_kalman_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("Kalman Demo", &example->show_kalman_window);
        ImGui::ShowImKalmanDemoWindow();
        ImGui::End();
    }

    // Show FFT Window
    if (example->show_fft_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 1024), ImGuiCond_FirstUseEver);
        ImGui::Begin("FFT Demo", &example->show_fft_window);
        ImGui::ShowImFFTDemoWindow();
        ImGui::End();
    }

    // Show STFT Window
    if (example->show_stft_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 1024), ImGuiCond_FirstUseEver);
        ImGui::Begin("STFT Demo", &example->show_stft_window);
        ImGui::ShowImSTFTDemoWindow();
        ImGui::End();
    }

    // Show ImMat line demo
    if (example->show_mat_draw_window)
    {
        ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        ImGui::Begin("ImMat draw Demo", &example->show_mat_draw_window);
        example->DrawLineDemo();
        ImGui::End();
    }

    // Show ImMat warp matrix demo
    if (example->show_mat_warp_matrix)
    {
        ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        ImGui::Begin("ImMat warp matrix Demo", &example->show_mat_warp_matrix);
        example->WarpMatrixDemo();
        ImGui::End();
    }

    // Show Text Edit Window
    if (example->show_text_editor_window)
    {
        example->editor.text_edit_demo(&example->show_text_editor_window);
    }

    // Show Tab Window
    if (example->show_tab_window)
    {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: TabWindow", &example->show_tab_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsTabWindow();   // see its code for further info         
        }
        ImGui::End();
    }

    // Show Node Editor Window
    if (example->show_node_editor_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024,1024), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: Node Editor", &example->show_node_editor_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowNodeEditorWindow();   // see its code for further info         
        }
        ImGui::End();
    }

    // Show Curve Demo Window
    if (example->show_curve_demo_window)
    {
        ImGui::SetNextWindowSize(ImVec2(800,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: Curve Demo", &example->show_curve_demo_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowCurveDemo();   // see its code for further info         
        }
        ImGui::End();
    }

    // Show Spline Demo Window
    if (example->show_spline_demo_window)
    {
        ImGui::SetNextWindowSize(ImVec2(800,800), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: Spline Demo", &example->show_spline_demo_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowSplineDemo();   // see its code for further info         
        }
        ImGui::End();
    }

    // Show Zmo Quat Window
    if (example->show_zmoquat_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1280, 900), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("ZMOQuat", &example->show_zmoquat_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowQuatDemo();
        }
        ImGui::End();
    }

    // Show Zmo Window
    if (example->show_zmo_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##ZMO", &example->show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGuizmo::ShowImGuiZmoDemo();
        ImGui::End();
    }

    // Show Toggle Window
    if (example->show_toggle_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##Toggle", &example->show_toggle_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGui::imgui_toggle_example();
        ImGui::End();
    }

    // Show TexInspect Window
    if (example->show_tex_inspect_window)
    {
        ImGuiTexInspect::ShowImGuiTexInspectDemo(&example->show_tex_inspect_window);
    }

    // Show ImCoolbar Window
    if (example->show_coolbar_window)
    {
        Show_Coolbar_demo_window();
    }

#if IMGUI_VULKAN_SHADER
    // Show Vulkan Shader Test Window
    if (example->show_shader_window)
    {
        example->ImVulkanTestWindow("ImGui Vulkan test", &example->show_shader_window, 0);
    }
#endif
    if (app_will_quit)
        app_done = true;
    return app_done;
}

bool Example_Splash_Screen(void* handle, bool app_will_quit)
{
    const int delay = 20;
    static int x = 0;
    auto& io = ImGui::GetIO();
    ImGuiCond cond = ImGuiCond_None;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | 
                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize, cond);
    ImGui::Begin("Content", nullptr, flags);
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32_WHITE);
    ImGui::SetWindowFontScale(2.0);
    std::string str = "Example Splash";
    auto mark_size = ImGui::CalcTextSize(str.c_str());
    float xoft = (io.DisplaySize.x - mark_size.x) / 2;
    float yoft = (io.DisplaySize.y - mark_size.y - 32) / 2;
    ImGui::GetWindowDrawList()->AddText(ImVec2(xoft, yoft), IM_COL32_BLACK, str.c_str());
    ImGui::SetWindowFontScale(1.0);

    ImGui::SetCursorPos(ImVec2(4, io.DisplaySize.y - 32));
    float progress = (float)x / (float)delay;
    ImGui::ProgressBar("##esplash_progress", progress, 0.f, 1.f, "", ImVec2(io.DisplaySize.x - 16, 8), 
                                ImVec4(0.3f, 0.3f, 0.8f, 1.f), ImVec4(0.1f, 0.1f, 0.3f, 1.f), ImVec4(0.f, 0.f, 0.8f, 1.f));
    ImGui::End();

    if (x < delay)
    {
        ImGui::sleep(1);
        x++;
        return false;
    }
    return true;
}

void Application_Setup(ApplicationWindowProperty& property)
{
    property.name = "Application_Example";
    property.font_scale = 2.0f;
    property.splash_screen_width = 600;
    property.splash_screen_height = 300;
    property.splash_screen_alpha = 0.9;
    property.application.Application_Initialize = Example_Initialize;
    property.application.Application_Finalize = Example_Finalize;
    property.application.Application_Frame = Example_Frame;
    property.application.Application_SplashScreen = Example_Splash_Screen;
}
