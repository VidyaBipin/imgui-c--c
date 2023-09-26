//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include <imgui.h>
#include <imgui_user.h>
//-----------------------------------------------------------------------------------------------------------------

#include "imgui_helper.h"
#include <errno.h>
#include <mutex>
#include <thread>
#include <sstream>
#include <iomanip>

#if __ARM_NEON
#include <arm_neon.h>
#elif __SSE__ || __AVX__
#include <neon2sse.h>
#endif // __ARM_NEON

#ifdef _WIN32
#include <windows.h>
#include <winerror.h>   // For SUCCEEDED macro
#include <shellapi.h>	// ShellExecuteA(...) - Shell32.lib
#include <objbase.h>    // CoInitializeEx(...)  - ole32.lib
#include <shlobj.h>     // For SHGetFolderPathW and various CSIDL "magic numbers"
#include <stringapiset.h>   // For WideCharToMultiByte
#include <psapi.h> 
#if IMGUI_RENDERING_DX11
struct IUnknown;
#include <d3d11.h>
#elif IMGUI_RENDERING_DX9
#include <d3d9.h>
#endif
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#define PATH_SEP '\\'
#define PATH_SETTINGS "\\AppData\\Roaming\\"
#else //_WIN32
#include <unistd.h>
#include <stdlib.h> // system
#include <pwd.h>
#include <sys/stat.h>
#include <sys/resource.h>
#define PATH_SEP '/'
#endif //_WIN32

#if defined(__APPLE__)
#define PATH_SETTINGS "/Library/Application Support/"
#include <mach/task.h>
#include <mach/mach_init.h>
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#define PATH_SETTINGS "/.config/"
#endif

#if defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_ES2               // Emscripten    -> GL ES 2, "#version 100"
#define PATH_SETTINGS "/.config/"
#endif

#include <vector>
#include <algorithm>

#if IMGUI_VULKAN_SHADER
#include "ImVulkanShader.h"
#endif

#if IMGUI_RENDERING_MATAL
#ifdef IMGUI_OPENGL
#undef IMGUI_OPENGL
#define IMGUI_OPENGL 0
#endif
#endif

#if IMGUI_OPENGL
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#if IMGUI_RENDERING_GL3
#include <imgui_impl_opengl3.h>
#elif IMGUI_RENDERING_GL2
#include <imgui_impl_opengl2.h>
#endif
#endif

#if !defined(alloca)
#	if defined(__GLIBC__) || defined(__sun) || defined(__APPLE__) || defined(__NEWLIB__)
#		include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#	elif defined(_WIN32)
#       include <malloc.h>     // alloca
#       if !defined(alloca)
#           define alloca _alloca  // for clang with MS Codegen
#       endif //alloca
#   elif defined(__GLIBC__) || defined(__sun)
#       include <alloca.h>     // alloca
#   else
#       include <stdlib.h>     // alloca
#   endif
#endif //alloca

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || \
	defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
	#define stat _stat
	#define stricmp _stricmp
	#include <cctype>
	// this option need c++17
	// Modify By Dicky
		#include <Windows.h>
		#include <dirent_portable.h> // directly open the dirent file attached to this lib
	// Modify By Dicky end
	#define PATH_SEP '\\'
	#ifndef PATH_MAX
		#define PATH_MAX 260
	#endif // PATH_MAX
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || \
	defined(__NetBSD__) || defined(__APPLE__) || defined (__EMSCRIPTEN__)
	#define stricmp strcasecmp
	#include <sys/types.h>
	// this option need c++17
	#ifndef USE_STD_FILESYSTEM
		#include <dirent.h> 
	#endif // USE_STD_FILESYSTEM
	#define PATH_SEP '/'
#endif

#if IMGUI_RENDERING_VULKAN
#include <imgui_impl_vulkan.h>
#endif

#if IMGUI_RENDERING_VULKAN
struct ImTexture
{
    ImTextureVk TextureID = nullptr;
    int     Width     = 0;
    int     Height    = 0;
    double  TimeStamp = NAN;
    std::thread::id CreateThread;
    bool NeedDestroy  = false;
};
#elif IMGUI_RENDERING_DX11
#include <imgui_impl_dx11.h>
struct ImTexture
{
    ID3D11ShaderResourceView * TextureID = nullptr;
    int    Width     = 0;
    int    Height    = 0;
    double  TimeStamp = NAN;
    std::thread::id CreateThread;
    bool NeedDestroy  = false;
};
#elif IMGUI_RENDERING_DX9
#include <imgui_impl_dx9.h>
struct ImTexture
{
    LPDIRECT3DTEXTURE9 TextureID = nullptr;
    int    Width     = 0;
    int    Height    = 0;
    double  TimeStamp = NAN;
    std::thread::id CreateThread;
    bool NeedDestroy  = false;
};
#elif IMGUI_OPENGL
struct ImTexture
{
    ImTextureGl TextureID = nullptr;
    int    Width     = 0;
    int    Height    = 0;
    double  TimeStamp = NAN;
    std::thread::id CreateThread;
    bool NeedDestroy  = false;
};
#else
struct ImTexture
{
    int    TextureID = -1;
    int    Width     = 0;
    int    Height    = 0;
    double  TimeStamp = NAN;
    std::thread::id CreateThread;
    bool NeedDestroy  = false;
};
#endif

namespace ImGui {

// ImGui Info
void ShowImGuiInfo()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    ImGui::Text("define: __cplusplus = %d", (int)__cplusplus);
    ImGui::Separator();
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
    ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
    ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
    ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
    ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
    ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
    ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
    ImGui::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
    ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
    ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
    ImGui::Text("define: __GNUC__ = %d", (int)__GNUC__);
#endif
#ifdef __clang_version__
    ImGui::Text("define: __clang_version__ = %s", __clang_version__);
#endif
    ImGui::Separator();
    ImGui::Text("Backend Platform Name: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
    ImGui::Text("Backend Renderer Name: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
#if IMGUI_RENDERING_VULKAN
    ImGui::Text("Backend GPU: %s", ImGui_ImplVulkan_GetDeviceName().c_str());
    ImGui::Text("Backend Vulkan API: %s", ImGui_ImplVulkan_GetApiVersion().c_str());
    ImGui::Text("Backend Vulkan Drv: %s", ImGui_ImplVulkan_GetDrvVersion().c_str());
    ImGui::Separator();
#elif IMGUI_OPENGL
#if IMGUI_RENDERING_GL3
    ImGui::Text("Gl Loader: %s", ImGui_ImplOpenGL3_GLLoaderName().c_str());
    ImGui::Text("GL Version: %s", ImGui_ImplOpenGL3_GetVerion().c_str());
#elif IMGUI_RENDERING_GL2
    ImGui::Text("Gl Loader: %s", ImGui_ImplOpenGL2_GLLoaderName().c_str());
    ImGui::Text("GL Version: %s", ImGui_ImplOpenGL2_GetVerion().c_str());
#endif
#endif
    ImGui::Text("Flash Timer: %.1f", io.ConfigMemoryCompactTimer >= 0.0f ? io.ConfigMemoryCompactTimer : 0);
    ImGui::Separator();
    ImGui::Text("Fonts: %d fonts", io.Fonts->Fonts.Size);
    ImGui::Text("Texure Size: %d x %d", io.Fonts->TexWidth, io.Fonts->TexHeight); 
    ImGui::Text("Display Size: %.2f x %.2f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Text("Display Framebuffer Scale: %.2f %.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
}
// Image Load
static std::vector<ImTexture> g_Textures;
std::mutex g_tex_mutex;

void ImGenerateOrUpdateTexture(ImTextureID& imtexid,int width,int height,int channels,const unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt,bool minFilterNearest,bool magFilterNearest,bool is_immat)
{
    IM_ASSERT(pixels);
    IM_ASSERT(channels>0 && channels<=4);
    unsigned char* data = nullptr;
#if IMGUI_RENDERING_VULKAN
    VkBuffer buffer {nullptr};
    size_t offset {0};
    bool is_vulkan = false;
    int bit_depth = 8;
    if (is_immat)
    {
        ImGui::ImMat* mat = (ImGui::ImMat*)pixels;
        if (mat->empty())
            return;
        bit_depth = mat->depth;
#if IMGUI_VULKAN_SHADER
        if (mat->device == IM_DD_VULKAN)
        {
            ImGui::VkMat* vkmat = (ImGui::VkMat*)mat;
            buffer = vkmat->buffer();
            offset = vkmat->buffer_offset();
            if (!buffer)
                return;
            is_vulkan = true;
        }
        else 
#endif
        if (mat->device == IM_DD_CPU)
        {
            data = (unsigned char *)mat->data;
            is_vulkan = false;
        }
    }
    if (!is_vulkan && !data)
        return;
    if (imtexid == 0)
    {
        // TODO::Dicky Need deal with 3 channels Image(link RGB / BGR) and 1 channel (Gray)
        g_tex_mutex.lock();
        g_Textures.resize(g_Textures.size() + 1);
        ImTexture& texture = g_Textures.back();
        if (is_vulkan)
            texture.TextureID = (ImTextureVk)ImGui_ImplVulkan_CreateTexture(buffer, offset, width, height, bit_depth);
        else
            texture.TextureID = (ImTextureVk)ImGui_ImplVulkan_CreateTexture(data, width, height, bit_depth);
        if (!texture.TextureID)
        {
            g_Textures.pop_back();
            g_tex_mutex.unlock();
            return;
        }
        texture.CreateThread = std::this_thread::get_id();
        texture.NeedDestroy = false;
        texture.Width  = width;
        texture.Height = height;
        imtexid = texture.TextureID;
        g_tex_mutex.unlock();
        return;
    }
#if IMGUI_VULKAN_SHADER
    if (is_vulkan)
        ImGui_ImplVulkan_UpdateTexture(imtexid, buffer, offset, width, height, bit_depth);
    else
#endif
        ImGui_ImplVulkan_UpdateTexture(imtexid, data, width, height, bit_depth);
#elif IMGUI_RENDERING_DX11
    auto textureID = (ID3D11ShaderResourceView *)imtexid;
    if (textureID)
    {
        textureID->Release();
        textureID = nullptr;
    }
    imtexid = ImCreateTexture(pixels, width, height);
#elif IMGUI_RENDERING_DX9
    LPDIRECT3DDEVICE9 pd3dDevice = (LPDIRECT3DDEVICE9)ImGui_ImplDX9_GetDevice();
    if (!pd3dDevice) return;
    LPDIRECT3DTEXTURE9& texid = reinterpret_cast<LPDIRECT3DTEXTURE9&>(imtexid);
    if (texid==0 && pd3dDevice->CreateTexture(width, height, useMipmapsIfPossible ? 0 : 1, 0, channels==1 ? D3DFMT_A8 : channels==2 ? D3DFMT_A8L8 : channels==3 ? D3DFMT_R8G8B8 : D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texid, NULL) < 0) return;

    D3DLOCKED_RECT tex_locked_rect;
    if (texid->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK) {texid->Release();texid=0;return;}
    if (channels==3 || channels==4) {
        unsigned char* pw;
        const unsigned char* ppxl = pixels;
        for (int y = 0; y < height; y++)    {
            pw = (unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y;  // each row has Pitch bytes
            ppxl = &pixels[y*width*channels];
            for( int x = 0; x < width; x++ )
            {
                *pw++ = ppxl[2];
                *pw++ = ppxl[1];
                *pw++ = ppxl[0];
                if (channels==4) *pw++ = ppxl[3];
                ppxl+=channels;
            }
        }
    }
    else {
        for (int y = 0; y < height; y++)    {
            memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * channels) * y, (width * channels));
        }
    }
    texid->UnlockRect(0);
#elif IMGUI_OPENGL
    glEnable(GL_TEXTURE_2D);
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

    if (imtexid == 0)
    {
        g_tex_mutex.lock();
        g_Textures.resize(g_Textures.size() + 1);
        ImTexture& texture = g_Textures.back();
        texture.TextureID = new ImTextureGL("GLTexture");
        glGenTextures(1, &texture.TextureID->gID);
        texture.CreateThread = std::this_thread::get_id();
        texture.NeedDestroy = false;
        texture.Width  = width;
        texture.Height = height;
        imtexid = texture.TextureID;
        g_tex_mutex.unlock();
    }

    auto textureID = (ImTextureGL *)imtexid;

    glBindTexture(GL_TEXTURE_2D, textureID->gID);

    GLenum clampEnum = 0x2900;    // 0x2900 -> GL_CLAMP; 0x812F -> GL_CLAMP_TO_EDGE
#   ifndef GL_CLAMP
#       ifdef GL_CLAMP_TO_EDGE
        clampEnum = GL_CLAMP_TO_EDGE;
#       else //GL_CLAMP_TO_EDGE
        clampEnum = 0x812F;
#       endif // GL_CLAMP_TO_EDGE
#   else //GL_CLAMP
    clampEnum = GL_CLAMP;
#   endif //GL_CLAMP

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wraps ? GL_REPEAT : clampEnum);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrapt ? GL_REPEAT : clampEnum);
    //const GLfloat borderColor[]={0.f,0.f,0.f,1.f};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
    if (magFilterNearest) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (useMipmapsIfPossible)   {
#       ifdef NO_IMGUI_OPENGL_GLGENERATEMIPMAP
#           ifndef GL_GENERATE_MIPMAP
#               define GL_GENERATE_MIPMAP 0x8191
#           endif //GL_GENERATE_MIPMAP
        // I guess this is compilable, even if it's not supported:
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);    // This call must be done before glTexImage2D(...) // GL_GENERATE_MIPMAP can't be used with NPOT if there are not supported by the hardware of GL_ARB_texture_non_power_of_two.
#       endif //NO_IMGUI_OPENGL_GLGENERATEMIPMAP
    }
    if (minFilterNearest) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmapsIfPossible ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
    else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmapsIfPossible ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    GLenum luminanceAlphaEnum = 0x190A; // 0x190A -> GL_LUMINANCE_ALPHA [Note that we're FORCING this definition even if when it's not defined! What should we use for 2 channels?]
    GLenum compressedLuminanceAlphaEnum = 0x84EB; // 0x84EB -> GL_COMPRESSED_LUMINANCE_ALPHA [Note that we're FORCING this definition even if when it's not defined! What should we use for 2 channels?]
#   ifdef GL_LUMINANCE_ALPHA
    luminanceAlphaEnum = GL_LUMINANCE_ALPHA;
#   endif //GL_LUMINANCE_ALPHA
#   ifdef GL_COMPRESSED_LUMINANCE_ALPHA
    compressedLuminanceAlphaEnum = GL_COMPRESSED_LUMINANCE_ALPHA;
#   endif //GL_COMPRESSED_LUMINANCE_ALPHA

#   ifdef IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY
    if (&imtexid==&gImImplPrivateParams.fontTex && channels==1) {
        GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        //printf("IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY used.\n");
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY
# ifdef __APPLE__
    GLenum grayFormat = GL_RED;
#else
    GLenum grayFormat = GL_ALPHA;
#endif
    GLenum ifmt = channels==1 ? grayFormat : channels==2 ? luminanceAlphaEnum : channels==3 ? GL_RGB : GL_RGBA;  // channels == 1 could be GL_LUMINANCE, GL_ALPHA, GL_RED ...
    GLenum fmt = ifmt;
#   ifdef IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE
    if (&imtexid==&gImImplPrivateParams.fontTex)    {
        ifmt = channels==1 ? GL_COMPRESSED_ALPHA : channels==2 ? compressedLuminanceAlphaEnum : channels==3 ? GL_COMPRESSED_RGB : GL_COMPRESSED_RGBA;  // channels == 1 could be GL_COMPRESSED_LUMINANCE, GL_COMPRESSED_ALPHA, GL_COMPRESSED_RED ...
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE

    if (is_immat)
    {
        ImGui::ImMat *mat = (ImGui::ImMat*)pixels;
#if IMGUI_VULKAN_SHADER
        if (mat->device == IM_DD_VULKAN)
        {
            ImGui::VkMat * vkmat = (ImGui::VkMat*)mat;
            if (!vkmat->empty())
            {
                auto data = ImGui::ImVulkanVkMatMapping(*vkmat);
                if (data) glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, mat->type == IM_DT_FLOAT32 ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
            }
        }
        else
#endif
        if (mat->device == IM_DD_CPU)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, mat->type == IM_DT_FLOAT32 ? GL_FLOAT : GL_UNSIGNED_BYTE, mat->data);
        }
    }
    else
        glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, pixels);

#   ifdef IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE
    if (&imtexid==&gImImplPrivateParams.fontTex)    {
        GLint compressed = GL_FALSE;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
        if (compressed==GL_FALSE)
            printf("Font texture compressed = %s\n",compressed==GL_TRUE?"true":"false");
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE

#   ifndef NO_IMGUI_OPENGL_GLGENERATEMIPMAP
    if (useMipmapsIfPossible) glGenerateMipmap(GL_TEXTURE_2D);
#   endif //NO_IMGUI_OPENGL_GLGENERATEMIPMAP
    glBindTexture(GL_TEXTURE_2D, last_texture);
#endif
    //fprintf(stderr, "[ImTexture]:%lu\n", g_Textures.size());
}

void ImCopyToTexture(ImTextureID& imtexid, unsigned char* pixels, int width, int height, int channels, int offset_x, int offset_y, bool is_immat)
{
    IM_ASSERT(imtexid);
    IM_ASSERT(pixels);
    IM_ASSERT(channels>0 && channels<=4);
    auto texture_width = ImGetTextureWidth(imtexid);
    auto texture_height = ImGetTextureHeight(imtexid);
    if (offset_x < 0 || offset_y < 0 ||
        offset_x + width > texture_width ||
        offset_y + height > texture_height)
        return;
#if IMGUI_RENDERING_VULKAN

    bool is_vulkan = false;
    int bit_depth = 8;
    VkBuffer buffer {nullptr};
    size_t offset {0};
    unsigned char* data = nullptr;
    if (is_immat)
    {
        ImGui::ImMat* mat = (ImGui::ImMat*)pixels;
        if (mat->empty())
            return;
        bit_depth = mat->depth;
#if IMGUI_VULKAN_SHADER
        if (mat->device == IM_DD_VULKAN)
        {
            ImGui::VkMat* vkmat = (ImGui::VkMat*)mat;
            buffer = vkmat->buffer();
            offset = vkmat->buffer_offset();
            if (!buffer)
                return;
            is_vulkan = true;
        }
        else 
#endif
        if (mat->device == IM_DD_CPU)
        {
            data = (unsigned char *)mat->data;
            is_vulkan = false;
        }
    }
    if (!is_vulkan && !data)
        return;
#if IMGUI_VULKAN_SHADER
    if (is_vulkan)
        ImGui_ImplVulkan_UpdateTexture(imtexid, buffer, offset, width, height, bit_depth, offset_x, offset_y);
    else
#endif
        ImGui_ImplVulkan_UpdateTexture(imtexid, data, width, height, bit_depth, offset_x, offset_y);
#elif IMGUI_RENDERING_DX11
#elif IMGUI_RENDERING_DX9
#elif IMGUI_OPENGL
    glEnable(GL_TEXTURE_2D);
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

    auto textureID = (ImTextureGL *)imtexid;
    glBindTexture(GL_TEXTURE_2D, textureID->gID);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    GLenum luminanceAlphaEnum = 0x190A; // 0x190A -> GL_LUMINANCE_ALPHA [Note that we're FORCING this definition even if when it's not defined! What should we use for 2 channels?]
    GLenum compressedLuminanceAlphaEnum = 0x84EB; // 0x84EB -> GL_COMPRESSED_LUMINANCE_ALPHA [Note that we're FORCING this definition even if when it's not defined! What should we use for 2 channels?]
#   ifdef GL_LUMINANCE_ALPHA
    luminanceAlphaEnum = GL_LUMINANCE_ALPHA;
#   endif //GL_LUMINANCE_ALPHA
#   ifdef GL_COMPRESSED_LUMINANCE_ALPHA
    compressedLuminanceAlphaEnum = GL_COMPRESSED_LUMINANCE_ALPHA;
#   endif //GL_COMPRESSED_LUMINANCE_ALPHA

#   ifdef IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY
    if (&imtexid==&gImImplPrivateParams.fontTex && channels==1) {
        GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        //printf("IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY used.\n");
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY
    GLenum ifmt = channels==1 ? GL_ALPHA : channels==2 ? luminanceAlphaEnum : channels==3 ? GL_RGB : GL_RGBA;  // channels == 1 could be GL_LUMINANCE, GL_ALPHA, GL_RED ...
    GLenum fmt = ifmt;

    if (is_immat)
    {
        ImGui::ImMat *mat = (ImGui::ImMat*)pixels;
        auto src_format = mat->type == IM_DT_FLOAT32 ? GL_FLOAT : GL_UNSIGNED_BYTE;
#if IMGUI_VULKAN_SHADER
        if (mat->device == IM_DD_VULKAN)
        {
            ImGui::VkMat * vkmat = (ImGui::VkMat*)mat;
            if (!vkmat->empty())
            {
                auto data = ImGui::ImVulkanVkMatMapping(*vkmat);
                if (data) glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, width, height, fmt, src_format, data);
            }
        }
        else
#endif
        if (mat->device == IM_DD_CPU)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, width, height, fmt, src_format, mat->data);
        }
    }
    else
        glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, width, height, fmt, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, last_texture);
#endif
}

ImTextureID ImCreateTexture(const void* data, int width, int height, double time_stamp, int bit_depth)
{
#if IMGUI_RENDERING_VULKAN
    g_tex_mutex.lock();
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();
    texture.TextureID = (ImTextureVk)ImGui_ImplVulkan_CreateTexture(data, width, height, bit_depth);
    if (!texture.TextureID)
    {
        g_Textures.pop_back();
        g_tex_mutex.unlock();
        return (ImTextureID)nullptr;
    }
    texture.CreateThread = std::this_thread::get_id();
    texture.NeedDestroy = false;
    texture.Width  = width;
    texture.Height = height;
    texture.TimeStamp = time_stamp;
    g_tex_mutex.unlock();
    return (ImTextureID)texture.TextureID;
#elif IMGUI_RENDERING_DX11
    ID3D11Device* pd3dDevice = (ID3D11Device*)ImGui_ImplDX11_GetDevice();
    if (!pd3dDevice)
        return nullptr;
    g_tex_mutex.lock();
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D *pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    //ID3D11ShaderResourceView * texture = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &texture.TextureID);
    pTexture->Release();
    texture.CreateThread = std::this_thread::get_id();
    texture.NeedDestroy = false;
    texture.Width  = width;
    texture.Height = height;
    texture.TimeStamp = time_stamp;
    g_tex_mutex.unlock();
    return (ImTextureID)texture.TextureID;
#elif IMGUI_RENDERING_DX9
    LPDIRECT3DDEVICE9 pd3dDevice = (LPDIRECT3DDEVICE9)ImGui_ImplDX9_GetDevice();
    if (!pd3dDevice)
        return nullptr;
    g_tex_mutex.lock();
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();
    if (pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture.TextureID, NULL) < 0)
    {
        g_tex_mutex.unlock();
        return nullptr;
    }
    D3DLOCKED_RECT tex_locked_rect;
    int bytes_per_pixel = 4;
    if (texture.TextureID->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
    {
        g_tex_mutex.unlock();
        return nullptr;
    }
    for (int y = 0; y < height; y++)
        memcpy((unsigned char*)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, (unsigned char* )data + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
    texture.TextureID->UnlockRect(0);
    texture.CreateThread = std::this_thread::get_id();
    texture.NeedDestroy = false;
    texture.Width  = width;
    texture.Height = height;
    texture.TimeStamp = time_stamp;
    g_tex_mutex.unlock();
    return (ImTextureID)texture.TextureID;
#elif IMGUI_OPENGL
    g_tex_mutex.lock();
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();
    texture.TextureID = new ImTextureGL("GLTexture");
    // Upload texture to graphics system
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &texture.TextureID->gID);
    glBindTexture(GL_TEXTURE_2D, texture.TextureID->gID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, last_texture);

    texture.CreateThread = std::this_thread::get_id();
    texture.NeedDestroy = false;
    texture.Width  = width;
    texture.Height = height;
    texture.TimeStamp = time_stamp;
    g_tex_mutex.unlock();
    return (ImTextureID)texture.TextureID;
#else
    return nullptr;
#endif
}

static std::vector<ImTexture>::iterator ImFindTexture(ImTextureID texture)
{
#if IMGUI_RENDERING_VULKAN
    auto textureID = reinterpret_cast<ImTextureVk>(texture);
#elif IMGUI_RENDERING_DX11
    auto textureID = (ID3D11ShaderResourceView *)texture;
#elif IMGUI_RENDERING_DX9
    auto textureID = reinterpret_cast<LPDIRECT3DTEXTURE9>(texture);
#elif IMGUI_OPENGL
    auto textureID = reinterpret_cast<ImTextureGl>(texture);
#else
    int textureID = -1;
#endif
    return std::find_if(g_Textures.begin(), g_Textures.end(), [textureID](ImTexture& texture)
    {
        return texture.TextureID == textureID;
    });
}

static void destroy_texture(ImTexture* tex)
{
#if IMGUI_RENDERING_VULKAN
    if (tex->TextureID)
    {
        ImGui_ImplVulkan_DestroyTexture(&tex->TextureID);
        tex->TextureID = nullptr;
    }
#elif IMGUI_RENDERING_DX11
    if (tex->TextureID)
    {
        tex->TextureID->Release();
        tex->TextureID = nullptr;
    }
#elif IMGUI_RENDERING_DX9
    if (tex->TextureID)
    {
        tex->TextureID->Release();
        tex->TextureID = nullptr;
    }
#elif IMGUI_OPENGL
    if (tex->TextureID)
    {
        glDeleteTextures(1, &tex->TextureID->gID);
        delete tex->TextureID;
        tex->TextureID = nullptr;
    }
#endif
}

void ImDestroyTexture(ImTextureID texture)
{
    //fprintf(stderr, "[Destroy ImTexture]:%lu\n", g_Textures.size());
    g_tex_mutex.lock();
    auto textureIt = ImFindTexture(texture);
    if (textureIt == g_Textures.end())
    {
        g_tex_mutex.unlock();
        return;
    }
    if (textureIt->CreateThread != std::this_thread::get_id())
    {
        textureIt->NeedDestroy = true;
        g_tex_mutex.unlock();
        return;
    }
    destroy_texture(&(*textureIt));
    g_Textures.erase(textureIt);
    g_tex_mutex.unlock();
}

void ImDestroyTextures()
{
    //fprintf(stderr, "[remain ImTexture]:%lu\n", g_Textures.size());
    g_tex_mutex.lock();
    for (auto iter = g_Textures.begin(); iter != g_Textures.end(); iter++)
    {
        destroy_texture(&(*iter));
    }
    g_Textures.clear();
    g_tex_mutex.unlock();
}

void ImUpdateTextures()
{
    g_tex_mutex.lock();
    for (auto iter = g_Textures.begin(); iter != g_Textures.end();)
    {
        if (!iter->NeedDestroy)
        {
            iter++;
        }
        else if (iter->CreateThread != std::this_thread::get_id())
        {
            iter++;
        }
        else
        {
            //fprintf(stderr, "[Update ImTexture delete]:%lu\n", g_Textures.size());
            destroy_texture(&(*iter));
            iter = g_Textures.erase(iter);
        }
    }
    g_tex_mutex.unlock();
    //fprintf(stderr, "[Update ImTexture]:%lu\n", g_Textures.size());
}

size_t ImGetTextureCount()
{
    return g_Textures.size();
}

int ImGetTextureWidth(ImTextureID texture)
{
    auto textureIt = ImFindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Width;
    return 0;
}

int ImGetTextureHeight(ImTextureID texture)
{
    auto textureIt = ImFindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Height;
    return 0;
}

double ImGetTextureTimeStamp(ImTextureID texture)
{
    auto textureIt = ImFindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->TimeStamp;
    return NAN;
}

ImTextureID ImLoadTexture(const char* path)
{
    int width = 0, height = 0, component = 0;
    if (auto data = stbi_load(path, &width, &height, &component, 4))
    {
        auto texture = ImCreateTexture(data, width, height);
        stbi_image_free(data);
        return texture;
    }
    else
        return nullptr;
}

void ImLoadImageToMat(const char* path, ImMat& mat, bool gray)
{
    int width = 0, height = 0, component = 0;
    if (auto data = stbi_load(path, &width, &height, &component, gray ? 1 : 4))
    {
        ImMat tmp;
        tmp.create_type(width, height, component, data, IM_DT_INT8);
        mat = tmp.clone();
        stbi_image_free(data);
    }
}

int ImGetTextureData(ImTextureID texture, void* data)
{
    int ret = -1;
    auto textureIt = ImFindTexture(texture);
    if (textureIt == g_Textures.end())
        return -1;
    if (!textureIt->TextureID || !data)
        return -1;

    int width = ImGui::ImGetTextureWidth(texture);
    int height = ImGui::ImGetTextureHeight(texture);
    int channels = 4; // TODO::Dicky need check

    if (width <= 0 || height <= 0 || channels <= 0)
        return -1;

#if IMGUI_RENDERING_VULKAN
    ret = ImGui_ImplVulkan_GetTextureData(textureIt->TextureID, data, width, height, channels);
#elif !IMGUI_EMSCRIPTEN && (IMGUI_RENDERING_GL3 || IMGUI_RENDERING_GL2)
    glEnable(GL_TEXTURE_2D);
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glBindTexture(GL_TEXTURE_2D, textureIt->TextureID->gID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    ret = 0;
#else
    return -1;
#endif
    if (ret != 0)
        return -1;
    return 0;
}

ImPixel ImGetTexturePixel(ImTextureID texture, float x, float y)
{
    ImPixel pixel = {};
    auto textureIt = ImFindTexture(texture);
    if (textureIt == g_Textures.end())
        return pixel;
    if (!textureIt->TextureID)
        return pixel;

    int width = ImGui::ImGetTextureWidth(texture);
    int height = ImGui::ImGetTextureHeight(texture);
    int channels = 4; // TODO::Dicky need check

    if (width <= 0 || height <= 0 || channels <= 0)
        return pixel;

    if (x < 0 || y < 0 || x > width || y > height)
        return pixel;

#if IMGUI_RENDERING_VULKAN
    auto color = ImGui_ImplVulkan_GetTexturePixel(textureIt->TextureID, x, y);
    pixel = {color.x, color.y, color.z, color.w};
#elif !IMGUI_EMSCRIPTEN && (IMGUI_RENDERING_GL3 || IMGUI_RENDERING_GL2)
    // ulgy using full texture data to pick one pixel
    // if GlVersion is greater then 4.5, maybe we can using glGetTextureSubImage
    void* data = IM_ALLOC(width * height * channels);
    int ret = ImGetTextureData(texture, data);
    if (ret == 0)
    {
        unsigned char * pixels = (unsigned char *)data;
        pixel.r = *(pixels + ((int)y * width + (int)x) * channels + 0) / 255.f;
        pixel.g = *(pixels + ((int)y * width + (int)x) * channels + 1) / 255.f;
        pixel.b = *(pixels + ((int)y * width + (int)x) * channels + 2) / 255.f;
        pixel.a = *(pixels + ((int)y * width + (int)x) * channels + 3) / 255.f;
    }
    if (data) IM_FREE(data);
#endif
    return pixel;
}

bool ImTextureToFile(ImTextureID texture, std::string path)
{
    int ret = -1;
    
    int width = ImGui::ImGetTextureWidth(texture);
    int height = ImGui::ImGetTextureHeight(texture);
    int channels = 4; // TODO::Dicky need check
    
    if (!width || !height || !channels)
    {
        return false;
    }

    void* data = IM_ALLOC(width * height * channels);
    ret = ImGetTextureData(texture, data);
    if (ret != 0)
    {
        IM_FREE(data);
        return false;
    }

    auto file_suffix = ImGuiHelper::path_filename_suffix(path);
    if (!file_suffix.empty())
    {
        if (file_suffix.compare(".png") == 0 || file_suffix.compare(".PNG") == 0)
            stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
        else if (file_suffix.compare(".jpg") == 0 || file_suffix.compare(".JPG") == 0 ||
                file_suffix.compare(".jpeg") == 0 || file_suffix.compare(".JPEG") == 0)
            stbi_write_jpg(path.c_str(), width, height, channels, data, width * channels);
        else if (file_suffix.compare(".bmp") == 0 || file_suffix.compare(".BMP") == 0)
            stbi_write_bmp(path.c_str(), width, height, channels, data);
        else if (file_suffix.compare(".tga") == 0 || file_suffix.compare(".TGA") == 0)
            stbi_write_tga(path.c_str(), width, height, channels, data);
    }
    else
    {
        path += ".png";
        stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    }
    if (data) IM_FREE(data);
    return true;
}

void ImMatToTexture(ImGui::ImMat mat, ImTextureID& texture)
{
    if (mat.empty())
        return;
    if (texture)
    {
        int image_width = ImGui::ImGetTextureWidth(texture);
        int image_height = ImGui::ImGetTextureHeight(texture);
        if (mat.w != image_width || mat.h != image_height)
        {
            // mat changed
            ImGui::ImDestroyTexture(texture);
            texture = nullptr;
        }
    }
    ImGui::ImGenerateOrUpdateTexture(texture, mat.w, mat.h, mat.c, (const unsigned char *)&mat, true);
}

void ImTextureToMat(ImTextureID texture, ImGui::ImMat& mat, ImVec2 offset, ImVec2 size)
{
    int ret = -1;
    if (!texture) return;
    int width = ImGui::ImGetTextureWidth(texture);
    int height = ImGui::ImGetTextureHeight(texture);
    int channels = 4; // TODO::Dicky need check
    
    if (!width || !height || !channels)
    {
        return;
    }

    if (offset.x == 0 && offset.y == 0 && (size.x == 0 || size.y == 0))
    {
        mat.create(width, height, channels, (size_t)1, 4);
        ret = ImGetTextureData(texture, mat.data);
        if (ret != 0) mat.release();
    }
    else
    {
        auto _size_x = ImMin((float)width - offset.x, size.x);
        auto _size_y = ImMin((float)height - offset.y, size.y);
        void* data = IM_ALLOC(width * height * channels);
        ret = ImGetTextureData(texture, data);
        if (ret != 0)
        {
            IM_FREE(data);
            return;
        }
        mat.create(_size_x, _size_y, channels, (size_t)1, 4);
        int line_size = _size_x * 4;
        for (int i = 0; i < _size_y; i++)
        {
            char * src_ptr = (char *)data + (int)((offset.y + i) * width * 4 + offset.x * 4);
            char * dst_ptr = (char *)mat.data + i * line_size;
            memcpy(dst_ptr, src_ptr, line_size);
        }
        IM_FREE(data);
    }
}

bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS)	
{
#       ifdef _WIN32
            //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);  // Needed ??? Well, let's suppose the user initializes it himself for now"
            return ((size_t)ShellExecuteA( NULL, exploreModeForWindowsOS ? "explore" : "open", url, "", ".", SW_SHOWNORMAL ))>32;
#       else //_WIN32
            if (exploreModeForWindowsOS) exploreModeForWindowsOS = false;   // No warnings
            char tmp[4096];
            const char* openPrograms[]={"xdg-open","gnome-open"};	// Not sure what can I append here for MacOS

            static int openProgramIndex=-2;
            if (openProgramIndex==-2)   {
                openProgramIndex=-1;
                for (size_t i=0,sz=sizeof(openPrograms)/sizeof(openPrograms[0]);i<sz;i++) {
                    strcpy(tmp,"/usr/bin/");	// Well, we should check all the folders inside $PATH... and we ASSUME that /usr/bin IS inside $PATH (see below)
                    strcat(tmp,openPrograms[i]);
                    FILE* fd = (FILE *)ImFileOpen(tmp,"r");
                    if (fd) {
                        fclose(fd);
                        openProgramIndex = (int)i;
                        //printf(stderr,"found %s\n",tmp);
                        break;
                    }
                }
            }

            // Note that here we strip the prefix "/usr/bin" and just use openPrograms[openProgramsIndex].
            // Also note that if nothing has been found we use "xdg-open" (that might still work if it exists in $PATH, but not in /usr/bin).
            strcpy(tmp,openPrograms[openProgramIndex<0?0:openProgramIndex]);

            strcat(tmp," \"");
            strcat(tmp,url);
            strcat(tmp,"\"");
            return system(tmp)==0;
#       endif //_WIN32
}

void CloseAllPopupMenus()   {
    ImGuiContext& g = *GImGui;
    while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();
}

// Posted by Omar in one post. It might turn useful...
bool IsItemActiveLastFrame()    {
    ImGuiContext& g = *GImGui;
    if (g.ActiveIdPreviousFrame)
        return g.ActiveIdPreviousFrame== g.LastItemData.ID;
    return false;
}
bool IsItemJustReleased()   {
    return IsItemActiveLastFrame() && !ImGui::IsItemActive();
}
bool IsItemDisabled()    {
    ImGuiContext& g = *GImGui;
    return (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == ImGuiItemFlags_Disabled;
}

void ShowTooltipOnHoverV(const char* fmt, va_list args)
{
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::BeginTooltip())
    {
        ImGui::TextV(fmt, args);
        ImGui::EndTooltip();
    }
}

void ShowTooltipOnHover(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ShowTooltipOnHoverV(fmt, args);
    va_end(args);
}

void Debug_DrawItemRect(const ImVec4& col)
{
    auto drawList = ImGui::GetWindowDrawList();
    auto itemMin = ImGui::GetItemRectMin();
    auto itemMax = ImGui::GetItemRectMax();
    drawList->AddRect(itemMin, itemMax, ImColor(col));
}

const ImFont *GetFont(int fntIndex) {return (fntIndex>=0 && fntIndex<ImGui::GetIO().Fonts->Fonts.size()) ? ImGui::GetIO().Fonts->Fonts[fntIndex] : NULL;}
void PushFont(int fntIndex)    {
    IM_ASSERT(fntIndex>=0 && fntIndex<ImGui::GetIO().Fonts->Fonts.size());
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[fntIndex]);
}
void TextColoredV(int fntIndex, const ImVec4 &col, const char *fmt, va_list args) {
    ImGui::PushFont(fntIndex);
    ImGui::TextColoredV(col,fmt, args);
    ImGui::PopFont();
}
void TextColored(int fntIndex, const ImVec4 &col, const char *fmt,...)  {
    va_list args;
    va_start(args, fmt);
    TextColoredV(fntIndex,col, fmt, args);
    va_end(args);
}
void TextV(int fntIndex, const char *fmt, va_list args) {
    if (ImGui::GetCurrentWindow()->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const char* text, *text_end;
    ImFormatStringToTempBufferV(&text, &text_end, fmt, args);
    ImGui::PushFont(fntIndex);
    TextUnformatted(text, text_end);
    ImGui::PopFont();
}
void Text(int fntIndex, const char *fmt,...)    {
    va_list args;
    va_start(args, fmt);
    TextV(fntIndex,fmt, args);
    va_end(args);
}

void AddTextComplex(ImDrawList *draw_list, const ImVec2 pos, const char * str, float font_size, ImU32 text_color, float outline_w, ImU32 outline_color, ImVec2 shadow_offset, ImU32 shadow_color)
{
    ImGui::SetWindowFontScale(font_size);
    ImGui::PushStyleVar(ImGuiStyleVar_TexGlyphShadowOffset, shadow_offset);
    ImGui::PushStyleColor(ImGuiCol_TexGlyphShadow, ImGui::ColorConvertU32ToFloat4(shadow_color));
    if (outline_w > 0)
    {
        draw_list->AddText(ImVec2(pos.x - outline_w, pos.y), outline_color, str);
        draw_list->AddText(ImVec2(pos.x, pos.y - outline_w), outline_color, str);
        draw_list->AddText(ImVec2(pos.x + outline_w, pos.y), outline_color, str);
        draw_list->AddText(ImVec2(pos.x, pos.y + outline_w), outline_color, str);
    }
    draw_list->AddText(pos, text_color, str);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::SetWindowFontScale(1.0);
}

void AddTextComplex(const char * str, float font_size, ImU32 text_color, float outline_w, ImU32 outline_color, ImVec2 shadow_offset, ImU32 shadow_color)
{
    AddTextComplex(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), str, font_size, text_color, outline_w, outline_color, shadow_offset, shadow_color);
}

bool GetTexCoordsFromGlyph(unsigned short glyph, ImVec2 &uv0, ImVec2 &uv1)
{
    if (!GImGui->Font) return false;
    const ImFontGlyph* g = GImGui->Font->FindGlyph(glyph);
    if (g)  {
        uv0.x = g->U0; uv0.y = g->V0;
        uv1.x = g->U1; uv1.y = g->V1;
        return true;
    }
    return false;
}

float CalcMainMenuHeight()
{
    // Warning: according to https://github.com/ocornut/imgui/issues/252 this approach can fail [Better call ImGui::GetWindowSize().y from inside the menu and store the result somewhere]
    if (GImGui->FontBaseSize>0) return GImGui->FontBaseSize + GImGui->Style.FramePadding.y * 2.0f;
    else {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        ImFont* font = ImGui::GetFont();
        if (!font) {
            if (io.Fonts->Fonts.size()>0) font = io.Fonts->Fonts[0];
            else return (14)+style.FramePadding.y * 2.0f;
        }
        return (io.FontGlobalScale * font->Scale * font->FontSize) + style.FramePadding.y * 2.0f;
    }
}

void RenderMouseCursor(const char* mouse_cursor, ImVec2 offset, float base_scale, int rotate, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow)
{
    ImGuiViewportP* viewport = (ImGuiViewportP*)ImGui::GetWindowViewport();
    ImDrawList* draw_list = ImGui::GetForegroundDrawList(viewport);
    ImGuiIO& io = ImGui::GetIO();
    const float FontSize = draw_list->_Data->FontSize;
    ImVec2 size(FontSize, FontSize);
    const ImVec2 pos = io.MousePos - offset;
    const float scale = base_scale * viewport->DpiScale;
    if (!viewport->GetMainRect().Overlaps(ImRect(pos, pos + ImVec2(size.x + 2, size.y + 2) * scale)))
        return;

    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    int rotation_start_index = draw_list->VtxBuffer.Size;
    draw_list->AddText(pos + ImVec2(-1, -1), col_border, mouse_cursor);
    draw_list->AddText(pos + ImVec2(1, 1), col_shadow, mouse_cursor);
    draw_list->AddText(pos, col_fill, mouse_cursor);
    if (rotate != 0)
    {
        float rad = M_PI / 180 * (90 - rotate);
        ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds
        auto& buf = draw_list->VtxBuffer;
        float s = sin(rad), c = cos(rad);
        for (int i = rotation_start_index; i < buf.Size; i++)
            l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);
        ImVec2 center = ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
        center = ImRotate(center, s, c) - center;
        
        for (int i = rotation_start_index; i < buf.Size; i++)
            buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
    }
}

// These two methods are inspired by imguidock.cpp
void PutInBackground(const char* optionalRootWindowName)
{
    ImGuiWindow* w = optionalRootWindowName ? FindWindowByName(optionalRootWindowName) : GetCurrentWindow();
    if (!w) return;
    ImGuiContext& g = *GImGui;
    if (g.Windows[0] == w) return;
    const int isz = g.Windows.Size;
    for (int i = 0; i < isz; i++)
    {
        if (g.Windows[i] == w)
        {
            for (int j = i; j > 0; --j) g.Windows[j] = g.Windows[j-1];  // shifts [0,j-1] to [1,j]
            g.Windows[0] = w;
            break;
        }
    }
}

void PutInForeground(const char* optionalRootWindowName)
{
    ImGuiWindow* w = optionalRootWindowName ? FindWindowByName(optionalRootWindowName) : GetCurrentWindow();
    if (!w) return;
    ImGuiContext& g = *GImGui;
    const int iszMinusOne = g.Windows.Size - 1;
    if (iszMinusOne<0 || g.Windows[iszMinusOne] == w) return;
    for (int i = iszMinusOne; i >= 0; --i)
    {
        if (g.Windows[i] == w)
        {
            for (int j = i; j < iszMinusOne; j++) g.Windows[j] = g.Windows[j+1];  // shifts [i+1,iszMinusOne] to [i,iszMinusOne-1]
            g.Windows[iszMinusOne] = w;
            break;
        }
    }
}

ScopedItemWidth::ScopedItemWidth(float width)
{
    ImGui::PushItemWidth(width);
}

ScopedItemWidth::~ScopedItemWidth()
{
    Release();
}

void ScopedItemWidth::Release()
{
    if (m_IsDone)
        return;

    ImGui::PopItemWidth();

    m_IsDone = true;
}

ScopedDisableItem::ScopedDisableItem(bool disable, float disabledAlpha)
    : m_Disable(disable)
{
    if (!m_Disable)
        return;

    ImGuiContext& g = *GImGui;
    auto wasDisabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == ImGuiItemFlags_Disabled;

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

    auto& stale = ImGui::GetStyle();
    m_LastAlpha = stale.Alpha;

    // Don't override alpha if we're already in disabled context.
    if (!wasDisabled)
        stale.Alpha = disabledAlpha;
}

ScopedDisableItem::~ScopedDisableItem()
{
    Release();
}

void ScopedDisableItem::Release()
{
    if (!m_Disable)
        return;

    auto& stale = ImGui::GetStyle();
    stale.Alpha = m_LastAlpha;

    ImGui::PopItemFlag();

    m_Disable = false;
}

ScopedSuspendLayout::ScopedSuspendLayout()
{
    m_Window = ImGui::GetCurrentWindow();
    m_CursorPos = m_Window->DC.CursorPos;
    m_CursorPosPrevLine = m_Window->DC.CursorPosPrevLine;
    m_CursorMaxPos = m_Window->DC.CursorMaxPos;
    m_CurrLineSize = m_Window->DC.CurrLineSize;
    m_PrevLineSize = m_Window->DC.PrevLineSize;
    m_CurrLineTextBaseOffset = m_Window->DC.CurrLineTextBaseOffset;
    m_PrevLineTextBaseOffset = m_Window->DC.PrevLineTextBaseOffset;
}

ScopedSuspendLayout::~ScopedSuspendLayout()
{
    Release();
}

void ScopedSuspendLayout::Release()
{
    if (m_Window == nullptr)
        return;

    m_Window->DC.CursorPos = m_CursorPos;
    m_Window->DC.CursorPosPrevLine = m_CursorPosPrevLine;
    m_Window->DC.CursorMaxPos = m_CursorMaxPos;
    m_Window->DC.CurrLineSize = m_CurrLineSize;
    m_Window->DC.PrevLineSize = m_PrevLineSize;
    m_Window->DC.CurrLineTextBaseOffset = m_CurrLineTextBaseOffset;
    m_Window->DC.PrevLineTextBaseOffset = m_PrevLineTextBaseOffset;

    m_Window = nullptr;
}

ItemBackgroundRenderer::ItemBackgroundRenderer(OnDrawCallback onDrawBackground)
    : m_OnDrawBackground(std::move(onDrawBackground))
{
    m_DrawList = ImGui::GetWindowDrawList();
    m_Splitter.Split(m_DrawList, 2);
    m_Splitter.SetCurrentChannel(m_DrawList, 1);
}

ItemBackgroundRenderer::~ItemBackgroundRenderer()
{
    Commit();
}

void ItemBackgroundRenderer::Commit()
{
    if (m_Splitter._Current == 0)
        return;

    m_Splitter.SetCurrentChannel(m_DrawList, 0);

    if (m_OnDrawBackground)
        m_OnDrawBackground(m_DrawList);

    m_Splitter.Merge(m_DrawList);
}

void ItemBackgroundRenderer::Discard()
{
    if (m_Splitter._Current == 1)
        m_Splitter.Merge(m_DrawList);
}

StorageHandler<MostRecentlyUsedList::Settings> MostRecentlyUsedList::s_Storage;


void MostRecentlyUsedList::Install(ImGuiContext* context)
{
    context->SettingsHandlers.push_back(s_Storage.MakeHandler("MostRecentlyUsedList"));

    s_Storage.ReadLine = [](ImGuiContext*, Settings* entry, const char* line)
    {
        const char* lineEnd = line + strlen(line);

        auto parseListEntry = [lineEnd](const char* line, int& index) -> const char*
        {
            char* indexEnd = nullptr;
            errno = 0;
            index = strtol(line, &indexEnd, 10);
            if (errno == ERANGE)
                return nullptr;
            if (indexEnd >= lineEnd)
                return nullptr;
            if (*indexEnd != '=')
                return nullptr;
            return indexEnd + 1;
        };


        int index = 0;
        if (auto path = parseListEntry(line, index))
        {
            if (static_cast<int>(entry->m_List.size()) <= index)
                entry->m_List.resize(index + 1);
            entry->m_List[index] = path;
        }
    };

    s_Storage.WriteAll = [](ImGuiContext*, ImGuiTextBuffer* out_buf, const StorageHandler<Settings>::Storage& storage)
    {
        for (auto& entry : storage)
        {
            out_buf->appendf("[%s][%s]\n", "MostRecentlyUsedList", entry.first.c_str());
            int index = 0;
            for (auto& value : entry.second->m_List)
                out_buf->appendf("%d=%s\n", index++, value.c_str());
            out_buf->append("\n");
        }
    };
}

MostRecentlyUsedList::MostRecentlyUsedList(const char* id, int capacity /*= 10*/)
    : m_ID(id)
    , m_Capacity(capacity)
    , m_List(s_Storage.FindOrCreate(id)->m_List)
{
}

void MostRecentlyUsedList::Add(const std::string& item)
{
    Add(item.c_str());
}

void MostRecentlyUsedList::Add(const char* item)
{
    auto itemIt = std::find(m_List.begin(), m_List.end(), item);
    if (itemIt != m_List.end())
    {
        // Item is already on the list. Rotate list to move it to the
        // first place.
        std::rotate(m_List.begin(), itemIt, itemIt + 1);
    }
    else
    {
        // Push new item to the back, rotate list to move it to the front,
        // pop back last element if we're over capacity.
        m_List.push_back(item);
        std::rotate(m_List.begin(), m_List.end() - 1, m_List.end());
        if (static_cast<int>(m_List.size()) > m_Capacity)
            m_List.pop_back();
    }

    PushToStorage();

    ImGui::MarkIniSettingsDirty();
}

void MostRecentlyUsedList::Clear()
{
    if (m_List.empty())
        return;

    m_List.resize(0);

    PushToStorage();

    ImGui::MarkIniSettingsDirty();
}

const std::vector<std::string>& MostRecentlyUsedList::GetList() const
{
    return m_List;
}

int MostRecentlyUsedList::Size() const
{
    return static_cast<int>(m_List.size());
}

void MostRecentlyUsedList::PullFromStorage()
{
    if (auto settings = s_Storage.Find(m_ID.c_str()))
        m_List = settings->m_List;
}

void MostRecentlyUsedList::PushToStorage()
{
    auto settings = s_Storage.FindOrCreate(m_ID.c_str());
    settings->m_List = m_List;
}

void Grid::Begin(const char* id, int columns, float width)
{
    Begin(ImGui::GetID(id), columns, width);
}

void Grid::Begin(ImU32 id, int columns, float width)
{
    m_CursorPos = ImGui::GetCursorScreenPos();

    ImGui::PushID(id);
    m_Columns = ImMax(1, columns);
    m_Storage = ImGui::GetStateStorage();

    for (int i = 0; i < columns; ++i)
    {
        ImGui::PushID(ColumnSeed());
        m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidthAcc"), -1.0f);
        ImGui::PopID();
    }

    m_ColumnAlignment = 0.0f;
    m_MinimumWidth = width;

    ImGui::BeginGroup();

    EnterCell(0, 0);
}

void Grid::NextColumn()
{
    LeaveCell();

    int nextColumn = m_Column + 1;
    int nextRow    = 0;
    if (nextColumn > m_Columns)
    {
        nextColumn -= m_Columns;
        nextRow    += 1;
    }

    auto cursorPos = m_CursorPos;
    for (int i = 0; i < nextColumn; ++i)
    {
        ImGui::PushID(ColumnSeed(i));
        auto maximumColumnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
        ImGui::PopID();

        if (maximumColumnWidth > 0.0f)
            cursorPos.x += maximumColumnWidth + ImGui::GetStyle().ItemSpacing.x;
    }

    ImGui::SetCursorScreenPos(cursorPos);

    EnterCell(nextColumn, nextRow);
}

void Grid::NextRow()
{
    LeaveCell();

    auto cursorPos = ImGui::GetCursorScreenPos();
    cursorPos.x = m_CursorPos.x;
    for (int i = 0; i < m_Column; ++i)
    {
        ImGui::PushID(ColumnSeed(i));
        auto maximumColumnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
        ImGui::PopID();

        if (maximumColumnWidth > 0.0f)
            cursorPos.x += maximumColumnWidth + ImGui::GetStyle().ItemSpacing.x;
    }

    ImGui::SetCursorScreenPos(cursorPos);

    EnterCell(m_Column, m_Row + 1);
}

void Grid::EnterCell(int column, int row)
{
    m_Column = column;
    m_Row    = row;

    ImGui::PushID(ColumnSeed());
    m_MaximumColumnWidthAcc = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidthAcc"), -1.0f);
    auto maximumColumnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
    ImGui::PopID();

    ImGui::PushID(Seed());
    auto lastCellWidth = m_Storage->GetFloat(ImGui::GetID("LastCellWidth"), -1.0f);

    if (maximumColumnWidth >= 0.0f && lastCellWidth >= 0.0f)
    {
        auto freeSpace = maximumColumnWidth - lastCellWidth;

        auto offset = ImFloor(m_ColumnAlignment * freeSpace);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(offset, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::PopStyleVar();
    }

    ImGui::BeginGroup();
}

void Grid::LeaveCell()
{
    ImGui::EndGroup();

    auto itemSize = ImGui::GetItemRectSize();
    m_Storage->SetFloat(ImGui::GetID("LastCellWidth"), itemSize.x);
    ImGui::PopID();

    m_MaximumColumnWidthAcc = ImMax(m_MaximumColumnWidthAcc, itemSize.x);
    ImGui::PushID(ColumnSeed());
    m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidthAcc"), m_MaximumColumnWidthAcc);
    ImGui::PopID();
}

void Grid::SetColumnAlignment(float alignment)
{
    alignment = ImClamp(alignment, 0.0f, 1.0f);
    m_ColumnAlignment = alignment;
}

void Grid::End()
{
    LeaveCell();

    ImGui::EndGroup();

    float totalWidth = 0.0f;
    for (int i = 0; i < m_Columns; ++i)
    {
        ImGui::PushID(ColumnSeed(i));
        auto currentMaxCellWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidthAcc"), -1.0f);
        totalWidth += currentMaxCellWidth;
        m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidth"), currentMaxCellWidth);
        ImGui::PopID();
    }

    if (totalWidth < m_MinimumWidth)
    {
        auto spaceToDivide = m_MinimumWidth - totalWidth;
        auto spacePerColumn = ImCeil(spaceToDivide / m_Columns);

        for (int i = 0; i < m_Columns; ++i)
        {
            ImGui::PushID(ColumnSeed(i));
            auto columnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
            columnWidth += spacePerColumn;
            m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidth"), columnWidth);
            ImGui::PopID();

            spaceToDivide -= spacePerColumn;
            if (spaceToDivide < 0)
                spacePerColumn += spaceToDivide;
        }
    }

    ImGui::PopID();
}
} // namespace Imgui

#include <stdio.h>  // FILE
namespace ImGuiHelper
{
static const char* FieldTypeNames[ImGui::FT_COUNT+1] = {"INT","UNSIGNED","FLOAT","DOUBLE","STRING","ENUM","BOOL","COLOR","TEXTLINE","CUSTOM","COUNT"};
static const char* FieldTypeFormats[ImGui::FT_COUNT]={"%d","%u","%f","%f","%s","%d","%d","%f","%s","%s"};
static const char* FieldTypeFormatsWithCustomPrecision[ImGui::FT_COUNT]={"%.*d","%*u","%.*f","%.*f","%*s","%*d","%*d","%.*f","%*s","%*s"};

void Deserializer::clear()
{
    if (f_data) ImGui::MemFree(f_data);
    f_data = NULL;f_size=0;
}

bool Deserializer::loadFromFile(const char *filename)
{
    clear();
    if (!filename) return false;
    FILE* f;
    if ((f = (FILE *)ImFileOpen(filename, "rt")) == NULL) return false;
    if (fseek(f, 0, SEEK_END))
    {
        fclose(f);
        return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)
    {
        fclose(f);
        return false;
    }
    f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))
    {
        fclose(f);
        return false;
    }
    f_data = (char*)ImGui::MemAlloc(f_size+1);
    f_size = fread(f_data, 1, f_size, f); // Text conversion alter read size so let's not be fussy about return value
    fclose(f);
    if (f_size == 0)
    {
        clear();
        return false;
    }
    f_data[f_size] = 0;
    ++f_size;
    return true;
}
bool Deserializer::allocate(size_t sizeToAllocate, const char *optionalTextToCopy, size_t optionalTextToCopySize)
{
    clear();
    if (sizeToAllocate==0) return false;
    f_size = sizeToAllocate;
    f_data = (char*)ImGui::MemAlloc(f_size);
    if (!f_data) {clear();return false;}
    if (optionalTextToCopy && optionalTextToCopySize>0) memcpy(f_data,optionalTextToCopy,optionalTextToCopySize>f_size ? f_size:optionalTextToCopySize);
    return true;
}
Deserializer::Deserializer(const char *filename) : f_data(NULL),f_size(0)
{
    if (filename) loadFromFile(filename);
}
Deserializer::Deserializer(const char *text, size_t textSizeInBytes) : f_data(NULL),f_size(0)
{
    allocate(textSizeInBytes,text,textSizeInBytes);
}

const char* Deserializer::parse(Deserializer::ParseCallback cb, void *userPtr, const char *optionalBufferStart) const
{
    if (!cb || !f_data || f_size==0) return NULL;
    //------------------------------------------------
    // Parse file in memory
    char name[128];name[0]='\0';
    char typeName[32];char format[32]="";bool quitParsing = false;
    char charBuffer[sizeof(double)*10];void* voidBuffer = (void*) &charBuffer[0];
    static char textBuffer[2050];
    const char* varName = NULL;int numArrayElements = 0;FieldType ft = ImGui::FT_COUNT;
    const char* buf_end = f_data + f_size-1;
    for (const char* line_start = optionalBufferStart ? optionalBufferStart : f_data; line_start < buf_end; )
    {
        const char* line_end = line_start;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r') line_end++;

        if (name[0]=='\0' && line_start[0] == '[' && line_end > line_start && line_end[-1] == ']')
        {
            ImFormatString(name, IM_ARRAYSIZE(name), "%.*s", (int)(line_end-line_start-2), line_start+1);
            //fprintf(stderr,"name: %s\n",name);  // dbg

            // Here we have something like: FLOAT-4:VariableName
            // We have to split into FLOAT 4 VariableName
            varName = NULL;numArrayElements = 0;ft = ImGui::FT_COUNT;format[0]='\0';
            const char* colonCh = strchr(name,':');
            const char* minusCh = strchr(name,'-');
            if (!colonCh)
            {
                fprintf(stderr,"ImGuiHelper::Deserializer::parse(...) warning (skipping line with no semicolon). name: %s\n",name);  // dbg
                name[0]='\0';
            }
            else
            {
                ptrdiff_t diff = 0,diff2 = 0;
                if (!minusCh || (minusCh-colonCh)>0)  {diff = (colonCh-name);numArrayElements=1;}
                else
                {
                    diff = (minusCh-name);
                    diff2 = colonCh-minusCh;
                    if (diff2>1 && diff2<7)
                    {
                        static char buff[8];
                        strncpy(&buff[0],(const char*) &minusCh[1],diff2);buff[diff2-1]='\0';
                        sscanf(buff,"%d",&numArrayElements);
                        //fprintf(stderr,"WARN: %s\n",buff);
                    }
                    else if (diff>0) numArrayElements = ((char)name[diff+1]-(char)'0');  // TODO: FT_STRING needs multibytes -> rewrite!
                }
                if (diff>0)
                {
                    const size_t len = (size_t)(diff>31?31:diff);
                    strncpy(typeName,name,len);typeName[len]='\0';

                    for (int t=0;t<=ImGui::FT_COUNT;t++)
                    {
                        if (strcmp(typeName,FieldTypeNames[t])==0)
                        {
                            ft = (FieldType) t;break;
                        }
                    }
                    varName = ++colonCh;

                    const bool isTextOrCustomType = ft==ImGui::FT_STRING || ft==ImGui::FT_TEXTLINE  || ft==ImGui::FT_CUSTOM;
                    if (ft==ImGui::FT_COUNT || numArrayElements<1 || (numArrayElements>4 && !isTextOrCustomType))
                    {
                        fprintf(stderr,"ImGuiHelper::Deserializer::parse(...) Error (wrong type detected): line:%s type:%d numArrayElements:%d varName:%s typeName:%s\n",name,(int)ft,numArrayElements,varName,typeName);
                        varName=NULL;
                    }
                    else
                    {
                        if (ft==ImGui::FT_STRING && varName && varName[0]!='\0')
                        {
                            if (numArrayElements==1 && (!minusCh || (minusCh-colonCh)>0))
                            {
                                numArrayElements=0;   // NEW! To handle blank strings ""
                            }
                            //Process soon here, as the string can be multiline
                            line_start = ++line_end;
                            //--------------------------------------------------------
                            int cnt = 0;
                            while (line_end < buf_end && cnt++ < numArrayElements-1) ++line_end;
                            textBuffer[0]=textBuffer[2049]='\0';
                            const int maxLen = numArrayElements>0 ? (cnt>2049?2049:cnt) : 0;
                            strncpy(textBuffer,line_start,maxLen+1);
                            textBuffer[maxLen]='\0';
                            quitParsing = cb(ft,numArrayElements,(void*)textBuffer,varName,userPtr);
                            //fprintf(stderr,"Deserializer::parse(...) value:\"%s\" to type:%d numArrayElements:%d varName:%s maxLen:%d\n",textBuffer,(int)ft,numArrayElements,varName,maxLen);  // dbg


                            //else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
                            //--------------------------------------------------------
                            ft = ImGui::FT_COUNT;name[0]='\0';varName=NULL; // mandatory                            

                        }
                        else if (!isTextOrCustomType)
                        {
                            format[0]='\0';
                            for (int t=0;t<numArrayElements;t++)
                            {
                                if (t>0) strcat(format," ");
                                strcat(format,FieldTypeFormats[ft]);
                            }
                            // DBG:
                            //fprintf(stderr,"Deserializer::parse(...) DBG: line:%s type:%d numArrayElements:%d varName:%s format:%s\n",name,(int)ft,numArrayElements,varName,format);  // dbg
                        }
                    }
                }
            }
        }
        else if (varName && varName[0]!='\0')
        {
            switch (ft)
            {
            case ImGui::FT_FLOAT:
            case ImGui::FT_COLOR:
            {
                float* p = (float*) voidBuffer;
                if ((numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                    (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                    (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                    (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                    quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_DOUBLE:
            {
                double* p = (double*) voidBuffer;
                if ((numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                    (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                    (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                    (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                    quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_INT:
            case ImGui::FT_ENUM:
            {
                int* p = (int*) voidBuffer;
                if ((numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                    (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                    (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                    (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                    quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_BOOL:
            {
                bool* p = (bool*) voidBuffer;
                int tmp[4];
                if ((numArrayElements==1 && sscanf(line_start, format, &tmp[0])==numArrayElements) ||
                    (numArrayElements==2 && sscanf(line_start, format, &tmp[0],&tmp[1])==numArrayElements) ||
                    (numArrayElements==3 && sscanf(line_start, format, &tmp[0],&tmp[1],&tmp[2])==numArrayElements) ||
                    (numArrayElements==4 && sscanf(line_start, format, &tmp[0],&tmp[1],&tmp[2],&tmp[3])==numArrayElements))    {
                    for (int i=0;i<numArrayElements;i++) p[i] = tmp[i];
                    quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                }
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_UNSIGNED:
            {
                unsigned* p = (unsigned*) voidBuffer;
                if ((numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                    (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                    (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                    (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                    quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_CUSTOM:
            case ImGui::FT_TEXTLINE:
            {
                // A similiar code can be used to parse "numArrayElements" line of text
                for (int i=0;i<numArrayElements;i++)
                {
                    textBuffer[0]=textBuffer[2049]='\0';
                    const int maxLen = (line_end-line_start)>2049?2049:(line_end-line_start);
                    if (maxLen<=0) break;
                    strncpy(textBuffer,line_start,maxLen+1);textBuffer[maxLen]='\0';
                    quitParsing = cb(ft,i,(void*)textBuffer,varName,userPtr);

                    //fprintf(stderr,"%d) \"%s\"\n",i,textBuffer);  // Dbg

                    if (quitParsing) break;
                    line_start = line_end+1;
                    line_end = line_start;
                    if (line_end == buf_end) break;
                    while (line_end < buf_end && *line_end != '\n' && *line_end != '\r') line_end++;
                }
            }
            break;
            default:
            fprintf(stderr,"Deserializer::parse(...) Warning missing value type:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            break;
            }
            //---------------------------------------------------------------------------------
            name[0]='\0';varName=NULL; // mandatory
        }

        line_start = line_end+1;

        if (quitParsing) return line_start;
    }

    //------------------------------------------------
    return buf_end;
}

bool GetFileContent(const char *filePath, ImVector<char> &contentOut, bool clearContentOutBeforeUsage, const char *modes, bool appendTrailingZeroIfModesIsNotBinary)
{
    ImVector<char>& f_data = contentOut;
    if (clearContentOutBeforeUsage) f_data.clear();
//----------------------------------------------------
    if (!filePath) return false;
    const bool appendTrailingZero = appendTrailingZeroIfModesIsNotBinary && modes && strlen(modes)>0 && modes[strlen(modes)-1]!='b';
    FILE* f;
    if ((f = (FILE *)ImFileOpen(filePath, modes)) == NULL) return false;
    if (fseek(f, 0, SEEK_END))
    {
        fclose(f);
        return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)
    {
        fclose(f);
        return false;
    }
    size_t f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))
    {
        fclose(f);
        return false;
    }
    f_data.resize(f_size+(appendTrailingZero?1:0));
    const size_t f_size_read = f_size>0 ? fread(&f_data[0], 1, f_size, f) : 0;
    fclose(f);
    if (f_size_read == 0 || f_size_read!=f_size)    return false;
    if (appendTrailingZero) f_data[f_size] = '\0';
//----------------------------------------------------
    return true;
}

bool SetFileContent(const char *filePath, const unsigned char* content, int contentSize,const char* modes)
{
    if (!filePath || !content) return false;
    FILE* f;
    if ((f = (FILE *)ImFileOpen(filePath, modes)) == NULL) return false;
    fwrite(content, contentSize, 1, f);
    fclose(f);f=NULL;
    return true;
}

class ISerializable
{
public:
    ISerializable() {}
    virtual ~ISerializable() {}
    virtual void close()=0;
    virtual bool isValid() const=0;
    virtual int print(const char* fmt, ...)=0;
    virtual int getTypeID() const=0;
};
class SerializeToFile : public ISerializable
{
public:
    SerializeToFile(const char* filename) : f(NULL)
    {
        saveToFile(filename);
    }
    SerializeToFile() : f(NULL) {}
    ~SerializeToFile() {close();}
    bool saveToFile(const char* filename)
    {
        close();
        f = (FILE *)ImFileOpen(filename,"w");
        return (f);
    }
    void close() {if (f) fclose(f);f=NULL;}
    bool isValid() const {return (f);}
    int print(const char* fmt, ...)
    {
        va_list args;va_start(args, fmt);
        const int rv = vfprintf(f,fmt,args);
        va_end(args);
        return rv;
    }
    int getTypeID() const {return 0;}
protected:
    FILE* f;
};
class SerializeToBuffer : public ISerializable
{
public:
    SerializeToBuffer(int initialCapacity=2048) {b.reserve(initialCapacity);b.resize(1);b[0]='\0';}
    ~SerializeToBuffer() {close();}
    bool saveToFile(const char* filename)
    {
        if (!isValid()) return false;
        return SetFileContent(filename,(unsigned char*)&b[0],b.size(),"w");
    }
    void close() {b.clear();ImVector<char> o;b.swap(o);b.resize(1);b[0]='\0';}
    bool isValid() const {return b.size()>0;}
    int print(const char* fmt, ...)
    {
        va_list args,args2;
        va_start(args, fmt);
        va_copy(args2,args);                                    // since C99 (MANDATORY! otherwise we must reuse va_start(args2,fmt): slow)
        const int additionalSize = vsnprintf(NULL,0,fmt,args);  // since C99
        va_end(args);
        //IM_ASSERT(additionalSize>0);

        const int startSz = b.size();
        b.resize(startSz+additionalSize);
        const int rv = vsnprintf(&b[startSz-1],additionalSize,fmt,args2);
        va_end(args2);
        //IM_ASSERT(additionalSize==rv);
        //IM_ASSERT(v[startSz+additionalSize-1]=='\0');

        return rv;
    }
    inline const char* getBuffer() const {return b.size()>0 ? &b[0] : NULL;}
    inline int getBufferSize() const {return b.size();}
    int getTypeID() const {return 1;}
protected:
    ImVector<char> b;
};
const char* Serializer::getBuffer() const
{
    return (f && f->getTypeID()==1 && f->isValid()) ? static_cast<SerializeToBuffer*>(f)->getBuffer() : NULL;
}
int Serializer::getBufferSize() const
{
    return (f && f->getTypeID()==1 && f->isValid()) ? static_cast<SerializeToBuffer*>(f)->getBufferSize() : 0;
}
bool Serializer::WriteBufferToFile(const char* filename,const char* buffer,int bufferSize)
{
    if (!buffer) return false;
    FILE* f = (FILE *)ImFileOpen(filename,"w");
    if (!f) return false;
    fwrite((void*) buffer,bufferSize,1,f);
    fclose(f);
    return true;
}

void Serializer::clear() {if (f) {f->close();}}
Serializer::Serializer(const char *filename)
{
    f=(SerializeToFile*) ImGui::MemAlloc(sizeof(SerializeToFile));
    IM_PLACEMENT_NEW((SerializeToFile*)f) SerializeToFile(filename);
}
Serializer::Serializer(int memoryBufferCapacity)
{
    f=(SerializeToBuffer*) ImGui::MemAlloc(sizeof(SerializeToBuffer));
    IM_PLACEMENT_NEW((SerializeToBuffer*)f) SerializeToBuffer(memoryBufferCapacity);
}
Serializer::~Serializer()
{
    if (f)
    {
        f->~ISerializable();
        ImGui::MemFree(f);
        f=NULL;
    }
}
template <typename T> inline static bool SaveTemplate(ISerializable* f,FieldType ft, const T* pValue, const char* name, int numArrayElements=1, int prec=-1)
{
    if (!f || ft==ImGui::FT_COUNT  || ft==ImGui::FT_CUSTOM || numArrayElements<0 || numArrayElements>4 || !pValue || !name || name[0]=='\0') return false;
    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);
    // value
    const char* precision = FieldTypeFormatsWithCustomPrecision[ft];
    for (int t=0;t<numArrayElements;t++)
    {
        if (t>0) f->print(" ");
        f->print(precision,prec,pValue[t]);
    }
    f->print("\n\n");
    return true;
}
bool Serializer::save(FieldType ft, const float* pValue, const char* name, int numArrayElements,  int prec)
{
    IM_ASSERT(ft==ImGui::FT_FLOAT || ft==ImGui::FT_COLOR);
    return SaveTemplate<float>(f,ft,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const double* pValue,const char* name,int numArrayElements, int prec)
{
    return SaveTemplate<double>(f,ImGui::FT_DOUBLE,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const bool* pValue,const char* name,int numArrayElements)
{
    if (!pValue || numArrayElements<0 || numArrayElements>4) return false;
    static int tmp[4];
    for (int i=0;i<numArrayElements;i++) tmp[i] = pValue[i] ? 1 : 0;
    return SaveTemplate<int>(f,ImGui::FT_BOOL,tmp,name,numArrayElements);
}
bool Serializer::save(FieldType ft,const int* pValue,const char* name,int numArrayElements, int prec)
{
    IM_ASSERT(ft==ImGui::FT_INT || ft==ImGui::FT_BOOL || ft==ImGui::FT_ENUM);
    if (prec==0) prec=-1;
    return SaveTemplate<int>(f,ft,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const unsigned* pValue,const char* name,int numArrayElements, int prec)
{
    if (prec==0) prec=-1;
    return SaveTemplate<unsigned>(f,ImGui::FT_UNSIGNED,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const char* pValue,const char* name,int pValueSize)
{
    FieldType ft = ImGui::FT_STRING;
    int numArrayElements = pValueSize;
    if (!f || ft==ImGui::FT_COUNT || !pValue || !name || name[0]=='\0') return false;
    numArrayElements = pValueSize;
    pValueSize=(int)strlen(pValue);if (numArrayElements>pValueSize || numArrayElements<=0) numArrayElements=pValueSize;
    if (numArrayElements<0) numArrayElements=0;

    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);
    // value
    f->print("%s\n\n",pValue);
    return true;
}
bool Serializer::saveTextLines(const char* pValue,const char* name)
{
    FieldType ft = ImGui::FT_TEXTLINE;
    if (!f || ft==ImGui::FT_COUNT || !pValue || !name || name[0]=='\0') return false;
    const char *tmp;const char *start = pValue;
    int left = strlen(pValue);int numArrayElements =0;  // numLines
    bool endsWithNewLine = pValue[left-1]=='\n';
    while ((tmp=strchr(start, '\n')))
    {
        ++numArrayElements;
        left-=tmp-start-1;
        start = ++tmp;  // to skip '\n'
    }
    if (left>0) ++numArrayElements;
    if (numArrayElements==0) return false;

    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);
    // value
    f->print("%s",pValue);
    if (!endsWithNewLine)  f->print("\n");
    f->print("\n");
    return true;
}
bool Serializer::saveTextLines(int numValues,bool (*items_getter)(void* data, int idx, const char** out_text),void* data,const char* name)
{
    FieldType ft = ImGui::FT_TEXTLINE;
    if (!items_getter || !f || ft==ImGui::FT_COUNT || numValues<=0 || !name || name[0]=='\0') return false;
    int numArrayElements =numValues;  // numLines

    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);

    // value
    const char* text=NULL;int len=0;
    for (int i=0;i<numArrayElements;i++)
    {
        if (items_getter(data,i,&text))
        {
            f->print("%s",text);
            if (len<=0 || text[len-1]!='\n')  f->print("\n");
        }
        else f->print("\n");
    }
    f->print("\n");
    return true;
}
bool Serializer::saveCustomFieldTypeHeader(const char* name, int numTextLines)
{
    // name
    f->print( "[%s",FieldTypeNames[ImGui::FT_CUSTOM]);
    if (numTextLines==0) numTextLines=1;
    if (numTextLines>1) f->print( "-%d",numTextLines);
    f->print( ":%s]\n",name);
    return true;
}

void StringSet(char *&destText, const char *text, bool allowNullDestText)
{
    if (destText) {ImGui::MemFree(destText);destText=NULL;}
    const char e = '\0';
    if (!text && !allowNullDestText) text=&e;
    if (text)
    {
        const int sz = strlen(text);
        destText = (char*) ImGui::MemAlloc(sz+1);strcpy(destText,text);
    }
}
void StringAppend(char *&destText, const char *textToAppend, bool allowNullDestText, bool prependLineFeedIfDestTextIsNotEmpty, bool mustAppendLineFeed)
{
    const int textToAppendSz = textToAppend ? strlen(textToAppend) : 0;
    if (textToAppendSz==0)
    {
        if (!destText && !allowNullDestText) {destText = (char*) ImGui::MemAlloc(1);strcpy(destText,"");}
        return;
    }
    const int destTextSz = destText ? strlen(destText) : 0;
    const bool mustPrependLF = prependLineFeedIfDestTextIsNotEmpty && (destTextSz>0);
    const bool mustAppendLF = mustAppendLineFeed;// && (destText);
    const int totalTextSz = textToAppendSz + destTextSz + (mustPrependLF?1:0) + (mustAppendLF?1:0);
    ImVector<char> totalText;totalText.resize(totalTextSz+1);
    totalText[0]='\0';
    if (destText)
    {
        strcpy(&totalText[0],destText);
        ImGui::MemFree(destText);destText=NULL;
    }
    if (mustPrependLF) strcat(&totalText[0],"\n");
    strcat(&totalText[0],textToAppend);
    if (mustAppendLF) strcat(&totalText[0],"\n");
    destText = (char*) ImGui::MemAlloc(totalTextSz+1);strcpy(destText,&totalText[0]);
}

int StringAppend(ImVector<char>& v,const char* fmt, ...)
{
    IM_ASSERT(v.size()>0 && v[v.size()-1]=='\0');
    va_list args,args2;

    va_start(args, fmt);
    va_copy(args2,args);                                    // since C99 (MANDATORY! otherwise we must reuse va_start(args2,fmt): slow)
    const int additionalSize = vsnprintf(NULL,0,fmt,args);  // since C99
    va_end(args);

    const int startSz = v.size();
    v.resize(startSz+additionalSize);
    const int rv = vsnprintf(&v[startSz-1],additionalSize,fmt,args2);
    va_end(args2);

    return rv;
}

// ImGui Theme Editor
static ImVec4 base = ImVec4(0.502f, 0.075f, 0.256f, 1.0f);
static ImVec4 bg   = ImVec4(0.200f, 0.220f, 0.270f, 1.0f);
static ImVec4 text = ImVec4(0.860f, 0.930f, 0.890f, 1.0f);
static float high_val = 0.8f;
static float mid_val = 0.5f;
static float low_val = 0.3f;
static float window_offset = -0.2f;
inline ImVec4 make_high(float alpha) 
{
    ImVec4 res(0, 0, 0, alpha);
    ImGui::ColorConvertRGBtoHSV(base.x, base.y, base.z, res.x, res.y, res.z);
    res.z = high_val;
    ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);
    return res;
}

inline ImVec4 make_mid(float alpha) 
{
    ImVec4 res(0, 0, 0, alpha);
    ImGui::ColorConvertRGBtoHSV(base.x, base.y, base.z, res.x, res.y, res.z);
    res.z = mid_val;
    ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);
    return res;
}

inline ImVec4 make_low(float alpha) 
{
    ImVec4 res(0, 0, 0, alpha);
    ImGui::ColorConvertRGBtoHSV(base.x, base.y, base.z, res.x, res.y, res.z);
    res.z = low_val;
    ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);
    return res;
}

inline ImVec4 make_bg(float alpha, float offset = 0.f) 
{
    ImVec4 res(0, 0, 0, alpha);
    ImGui::ColorConvertRGBtoHSV(bg.x, bg.y, bg.z, res.x, res.y, res.z);
    res.z += offset;
    ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);
    return res;
}

inline ImVec4 make_text(float alpha) 
{
    return ImVec4(text.x, text.y, text.z, alpha);
}

void ThemeGenerator(const char* name, bool* p_open, ImGuiWindowFlags flags) 
{
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin(name, p_open, flags);
    ImGui::ColorEdit3("base", (float*) &base, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorEdit3("bg", (float*) &bg, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorEdit3("text", (float*) &text, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::SliderFloat("high", &high_val, 0, 1);
    ImGui::SliderFloat("mid", &mid_val, 0, 1);
    ImGui::SliderFloat("low", &low_val, 0, 1);
    ImGui::SliderFloat("window", &window_offset, -0.4f, 0.4f);

    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text]                  = make_text(0.78f);
    style.Colors[ImGuiCol_TextDisabled]          = make_text(0.28f);
    style.Colors[ImGuiCol_WindowBg]              = make_bg(1.00f, window_offset);
    style.Colors[ImGuiCol_ChildBg]               = make_bg(0.58f);
    style.Colors[ImGuiCol_PopupBg]               = make_bg(0.9f);
    style.Colors[ImGuiCol_Border]                = make_bg(0.6f, -0.05f);
    style.Colors[ImGuiCol_BorderShadow]          = make_bg(0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg]               = make_bg(1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = make_mid(0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = make_mid(1.00f);
    style.Colors[ImGuiCol_TitleBg]               = make_low(1.00f);
    style.Colors[ImGuiCol_TitleBgActive]         = make_high(1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = make_bg(0.75f);
    style.Colors[ImGuiCol_MenuBarBg]             = make_bg(0.47f);
    style.Colors[ImGuiCol_ScrollbarBg]           = make_bg(1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = make_low(1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = make_mid(0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = make_mid(1.00f);
    style.Colors[ImGuiCol_CheckMark]             = make_high(1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = make_bg(1.0f, .1f);
    style.Colors[ImGuiCol_SliderGrabActive]      = make_high(1.0f);
    style.Colors[ImGuiCol_Button]                = make_bg(1.0f, .2f);
    style.Colors[ImGuiCol_ButtonHovered]         = make_mid(1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = make_high(1.00f);
    style.Colors[ImGuiCol_Header]                = make_mid(0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = make_mid(0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = make_high(1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = make_mid(0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = make_mid(1.00f);
    style.Colors[ImGuiCol_PlotLines]             = make_text(0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = make_mid(1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = make_text(0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = make_mid(1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = make_mid(0.43f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = make_bg(0.73f);
    style.Colors[ImGuiCol_Tab]                   = make_bg(0.40f);
    style.Colors[ImGuiCol_TabHovered]            = make_high(1.00f);
    style.Colors[ImGuiCol_TabActive]             = make_mid(1.00f);
    style.Colors[ImGuiCol_TabUnfocused]          = make_bg(0.40f);
    style.Colors[ImGuiCol_TabUnfocusedActive]    = make_bg(0.70f);
    //style.Colors[ImGuiCol_PlotLines]             = 
    //style.Colors[ImGuiCol_PlotLinesHovered]      = 
    //style.Colors[ImGuiCol_PlotHistogram]         = 
    //style.Colors[ImGuiCol_PlotHistogramHovered]  = 
    //style.Colors[ImGuiCol_TableHeaderBg]         = 
    //style.Colors[ImGuiCol_TableBorderStrong]     = 
    //style.Colors[ImGuiCol_TableBorderLight]      = 
    //style.Colors[ImGuiCol_TableRowBg]            = 
    //style.Colors[ImGuiCol_TableRowBgAlt]         = 
    //style.Colors[ImGuiCol_TextSelectedBg]        = 
    //style.Colors[ImGuiCol_DragDropTarget]        = 
    //style.Colors[ImGuiCol_NavHighlight]          = 
    //style.Colors[ImGuiCol_NavWindowingHighlight] =
    //style.Colors[ImGuiCol_NavWindowingDimBg]     = 
    //style.Colors[ImGuiCol_ModalWindowDimBg]      = 

    if (ImGui::Button("Export")) 
    {
        ImGui::LogToTTY();
        ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;\n");
        for (int i = 0; i < ImGuiCol_COUNT; i++) 
        {
            const ImVec4& col = style.Colors[i];
            const char* name = ImGui::GetStyleColorName(i);
            ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);\n",
                            name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
        }
        ImGui::LogFinish();
    }
    ImGui::End();
}

// System Toolkit
bool file_exists(const std::string& path)
{
    if (path.empty())
        return false;

    return access(path.c_str(), R_OK) == 0;
}

std::string path_url(const std::string& path)
{
    auto pos = path.find_last_of(PATH_SEP);
    if (pos != std::string::npos)
        return path.substr(0, pos + 1);
    return "";
}

std::string path_parent(const std::string& path)
{
    auto pos = path.find_last_of(PATH_SEP);
    if (pos != std::string::npos)
    {
        auto sub_path = path.substr(0, pos);
        pos = sub_path.find_last_of(PATH_SEP);
        if (pos != std::string::npos)
            return sub_path.substr(0, pos + 1);
    }
    return "";
}

std::string path_filename(const std::string& path)
{
    auto pos = path.find_last_of(PATH_SEP);
    if (pos != std::string::npos)
        return path.substr(pos + 1);
    return "";
}

std::string path_filename_prefix(const std::string& path)
{
    auto filename = path_filename(path);
    if (!filename.empty())
    {
        auto pos = filename.find_last_of('.');
        if (pos != std::string::npos)
            return filename.substr(0, pos);
    }
    return "";
}

std::string path_filename_suffix(const std::string& path)
{
    auto filename = path_filename(path);
    if (!filename.empty())
    {
        auto pos = filename.find_last_of('.');
        if (pos != std::string::npos)
            return filename.substr(pos);
    }
    return "";
}

std::string date_time_string()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);
    tm* datetime = localtime(&t);

    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << std::to_string(datetime->tm_year + 1900);
    oss << std::setw(2) << std::setfill('0') << std::to_string(datetime->tm_mon + 1);
    oss << std::setw(2) << std::setfill('0') << std::to_string(datetime->tm_mday );
    oss << std::setw(2) << std::setfill('0') << std::to_string(datetime->tm_hour );
    oss << std::setw(2) << std::setfill('0') << std::to_string(datetime->tm_min );
    oss << std::setw(2) << std::setfill('0') << std::to_string(datetime->tm_sec );
    oss << std::setw(3) << std::setfill('0') << std::to_string(millis);

    // fixed length string (17 chars) YYYYMMDDHHmmssiii
    return oss.str();
}

std::string username()
{
    std::string userName;
#ifdef _WIN32
    CHAR cUserNameBuffer[256];
    DWORD dwUserNameSize = 256;
    if(GetUserName(cUserNameBuffer, &dwUserNameSize))
    {
        userName = std::string(cUserNameBuffer);
    }
#else
    // try the system user info
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
        userName = std::string(pwd->pw_name);
    else
        // try the $USER environment variable
        userName = std::string(getenv("USER"));
#endif
    return userName;
}

std::string home_path()
{
    std::string homePath;
#ifdef _WIN32
    std::string homeDrive = std::string(getenv("HOMEDRIVE"));
    homePath = homeDrive + std::string(getenv("HOMEPATH"));
#else
    // try the system user info
    // NB: avoids depending on changes of the $HOME env. variable
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
        homePath = std::string(pwd->pw_dir);
    else
        // try the $HOME environment variable
        homePath = std::string(getenv("HOME"));
#endif
    return homePath + PATH_SEP;
}

bool create_directory(const std::string& path)
{
#ifdef _WIN32
    return !mkdir(path.c_str()) || errno == EEXIST;
#else
    return !mkdir(path.c_str(), 0755) || errno == EEXIST;
#endif
}

std::string settings_path(std::string app_name)
{
    // start from home folder
    // NB: use the env.variable $HOME to allow system to specify
    // another directory (e.g. inside a snap)
#ifdef _WIN32
    std::string homeDrive = std::string(getenv("HOMEDRIVE"));
    std::string home = homeDrive + std::string(getenv("HOMEPATH"));
#else
    std::string home(getenv("HOME"));
#endif
    // 2. try to access user settings folder
    std::string settingspath = home + PATH_SETTINGS;
    if (file_exists(settingspath))
    {
        // good, we have a place to put the settings file
        // settings should be in 'app_name' subfolder
        settingspath += app_name;

        // 3. create the vmix subfolder in settings folder if not existing already
        if ( !file_exists(settingspath))
        {
            if ( !create_directory(settingspath) )
                // fallback to home if settings path cannot be created
                settingspath = home;
        }
        return settingspath + PATH_SEP;
    }
    else
    {
        // fallback to home if settings path does not exists
        return home + PATH_SEP;
    }
}

std::string temp_path()
{
    std::string temp;
#ifdef _WIN32
    const char *tmpdir = getenv("TEMP");
#else
    const char *tmpdir = getenv("TMPDIR");
#endif
    if (tmpdir)
        temp = std::string(tmpdir);
    else
        temp = std::string( P_tmpdir );

    temp += PATH_SEP;
    return temp;
}

std::string cwd_path()
{
    char mCwdPath[PATH_MAX] = {0};

    if (getcwd(mCwdPath, sizeof(mCwdPath)) != NULL)
        return std::string(mCwdPath) + PATH_SEP;
    else
        return std::string();
}

std::string exec_path()
{
    std::string path = std::string(); 
    // Preallocate PATH_MAX (e.g., 4096) characters and hope the executable path isn't longer (including null byte)
    char exePath[PATH_MAX];
#if defined(__linux__)
    // Return written bytes, indicating if memory was sufficient
    int len = readlink("/proc/self/exe", exePath, PATH_MAX);
    if (len <= 0 || len == PATH_MAX) // memory not sufficient or general error occured
        return path;
    path = path_url(std::string(exePath));
#elif defined(_WIN32)
    // Return written bytes, indicating if memory was sufficient
    unsigned int len = GetModuleFileNameA(GetModuleHandleA(0x0), exePath, MAX_PATH);
    if (len == 0) // memory not sufficient or general error occured
        return path;
    path = path_url(std::string(exePath));
#elif defined(__APPLE__)
    unsigned int len = (unsigned int)PATH_MAX;
    // Obtain executable path to canonical path, return zero on success
    if (_NSGetExecutablePath(exePath, &len) == 0)
    {
        // Convert executable path to canonical path, return null pointer on error
        char * realPath = realpath(exePath, 0x0);
        if (realPath == 0x0)
            return path;
        path = path_url(std::string(realPath));
        free(realPath);
    }
    else // len is initialized with the required number of bytes (including zero byte)
    {
        char * intermediatePath = (char *)malloc(sizeof(char) * len);
        // Convert executable path to canonical path, return null pointer on error
        if (_NSGetExecutablePath(intermediatePath, &len) != 0)
        {
            free(intermediatePath);
            return path;
        }
        char * realPath = realpath(intermediatePath, 0x0);
        free(intermediatePath);
        // Check if conversion to canonical path succeeded
        if (realPath == 0x0)
            return path;
        path = path_url(std::string(realPath));
        free(realPath);
    }
#endif
    return path;
}

void execute(const std::string& command)
{
    int ignored __attribute__((unused));
#ifdef _WIN32
    ShellExecuteA( nullptr, nullptr, command.c_str(), nullptr, nullptr, 0 );
#elif defined(__APPLE__)
    (void) system( command.c_str() );
#else
    ignored = system( command.c_str() );
#endif
}

size_t memory_usage()
{
#if defined(__linux__)
    // Grabbing info directly from the /proc pseudo-filesystem.  Reading from
    // /proc/self/statm gives info on your own process, as one line of
    // numbers that are: virtual mem program size, resident set size,
    // shared pages, text/code, data/stack, library, dirty pages.  The
    // mem sizes should all be multiplied by the page size.
    size_t size = 0;
    FILE *file = fopen("/proc/self/statm", "r");
    if (file)
    {
        unsigned long m = 0;
        int ret = 0, ret2 = 0;
        ret = fscanf (file, "%lu", &m);  // virtual mem program size,
        ret2 = fscanf (file, "%lu", &m);  // resident set size,
        fclose (file);
        if (ret>0 && ret2>0)
            size = (size_t)m * getpagesize();
    }
    return size;

#elif defined(__APPLE__)
    // Inspired from
    // http://miknight.blogspot.com/2005/11/resident-set-size-in-mac-os-x.html
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    return t_info.resident_size;

#elif defined(_WIN32)
    // According to MSDN...
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo (GetCurrentProcess(), &counters, sizeof (counters)))
        return counters.PagefileUsage;
    else return 0;

#else
    return 0;
#endif
}

size_t memory_max_usage()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo (GetCurrentProcess(), &counters, sizeof (counters)))
        return counters.PeakPagefileUsage;
    else return 0;
#else
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);
    return 1024 * r_usage.ru_maxrss;
#endif
}

std::string MillisecToString(int64_t millisec, int show_millisec)
{
    std::ostringstream oss;
    if (millisec < 0)
    {
        oss << "-";
        millisec = -millisec;
    }
    uint64_t t = (uint64_t) millisec;
    uint32_t milli = (uint32_t)(t%1000); t /= 1000;
    uint32_t sec = (uint32_t)(t%60); t /= 60;
    uint32_t min = (uint32_t)(t%60); t /= 60;
    uint32_t hour = (uint32_t)t;
    if (hour > 0)
    {
        oss << std::setfill('0') << std::setw(2) << hour << ':'
            << std::setw(2) << min << ':'
            << std::setw(2) << sec;
        if (show_millisec == 3)
            oss << '.' << std::setw(3) << milli;
        else if (show_millisec == 2)
            oss << '.' << std::setw(2) << milli / 10;
        else if (show_millisec == 1)
            oss << '.' << std::setw(1) << milli / 100;
    }
    else
    {
        oss << std::setfill('0') << std::setw(2) << min << ':'
            << std::setw(2) << sec;
        if (show_millisec == 3)
            oss << '.' << std::setw(3) << milli;
        else if (show_millisec == 2)
            oss << '.' << std::setw(2) << milli / 10;
        else if (show_millisec == 1)
            oss << '.' << std::setw(1) << milli / 100;
    }
    return oss.str();
}
} //namespace ImGuiHelper

namespace base64 
{
// Decoder here
extern "C"
{
typedef enum {step_a, step_b, step_c, step_d} base64_decodestep;
typedef struct {base64_decodestep step;char plainchar;} base64_decodestate;

inline int base64_decode_value(char value_in)
{
	static const char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
	static const char decoding_size = sizeof(decoding);
	value_in -= 43;
	if (value_in < 0 || value_in > decoding_size) return -1;
	return decoding[(int)value_in];
}
inline void base64_init_decodestate(base64_decodestate* state_in)
{
	state_in->step = step_a;
	state_in->plainchar = 0;
}
inline int base64_decode_block(const char* code_in, const int length_in, char* plaintext_out, base64_decodestate* state_in)
{
	const char* codechar = code_in;
	char* plainchar = plaintext_out;
	char fragment;
	
	*plainchar = state_in->plainchar;
	
	switch (state_in->step)
	{
		while (1)
		{
        case step_a:
			do
            {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_a;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar    = (fragment & 0x03f) << 2;
        case step_b:
			do
            {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_b;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (fragment & 0x030) >> 4;
			*plainchar    = (fragment & 0x00f) << 4;
        case step_c:
			do
            {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_c;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (fragment & 0x03c) >> 2;
			*plainchar    = (fragment & 0x003) << 6;
        case step_d:
			do
            {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_d;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++   |= (fragment & 0x03f);
		}
	}
	/* control should not reach here */
	return plainchar - plaintext_out;
}
}	// extern "C"
struct decoder
{
	base64_decodestate _state;
	int _buffersize;
	
	decoder(int buffersize_in = 4096) : _buffersize(buffersize_in) {}
	int decode(char value_in) {return base64_decode_value(value_in);}
	int decode(const char* code_in, const int length_in, char* plaintext_out)	{return base64_decode_block(code_in, length_in, plaintext_out, &_state);}
};

// Encoder Here
extern "C"
{
typedef enum {step_A, step_B, step_C} base64_encodestep;
typedef struct {base64_encodestep step;char result;int stepcount;} base64_encodestate;

const int CHARS_PER_LINE = 2147483647;//72; // This was hard coded to 72 originally. But here we add '\n' at a later step. So we use MAX_INT here.
inline void base64_init_encodestate(base64_encodestate* state_in)
{
	state_in->step = step_A;
	state_in->result = 0;
	state_in->stepcount = 0;
}
inline char base64_encode_value(char value_in)
{
	static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (value_in > 63) return '=';
	return encoding[(int)value_in];
}
inline int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in)
{
	const char* plainchar = plaintext_in;
	const char* const plaintextend = plaintext_in + length_in;
	char* codechar = code_out;
    char result = '\0';
    char fragment = '\0';
	
	result = state_in->result;
	
	switch (state_in->step)
	{
		while (1)
		{
        case step_A:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_A;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result = (fragment & 0x0fc) >> 2;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x003) << 4;
        case step_B:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_B;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0f0) >> 4;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x00f) << 2;
        case step_C:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_C;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0c0) >> 6;
			*codechar++ = base64_encode_value(result);
			result  = (fragment & 0x03f) >> 0;
			*codechar++ = base64_encode_value(result);
			
			++(state_in->stepcount);
			if (state_in->stepcount == CHARS_PER_LINE/4)
			{
				*codechar++ = '\n';
				state_in->stepcount = 0;
			}
		}
	}
	/* control should not reach here */
	return codechar - code_out;
}

inline int base64_encode_blockend(char* code_out, base64_encodestate* state_in)
{
	char* codechar = code_out;
	
	switch (state_in->step)
	{
	case step_B:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		*codechar++ = '=';
		break;
	case step_C:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		break;
	case step_A:
		break;
	}
	*codechar++ = '\n';
	
	return codechar - code_out;
}	
} // extern "C"
struct encoder
{
	base64_encodestate _state;
	int _buffersize;
	
	encoder(int buffersize_in = 4096)
	: _buffersize(buffersize_in)
	{}
	int encode(char value_in)
	{
		return base64_encode_value(value_in);
	}
	int encode(const char* code_in, const int length_in, char* plaintext_out)
	{
		return base64_encode_block(code_in, length_in, plaintext_out, &_state);
	}
	int encode_end(char* plaintext_out)
	{
		return base64_encode_blockend(plaintext_out, &_state);
	}
};
} // namespace base64

namespace ImGui
{
namespace Stringifier
{
template <typename VectorChar> static bool Base64Decode(const char* input,VectorChar& output)
{
    output.clear();if (!input) return false;
    const int N = 4096;
    base64::decoder d(N);
    base64_init_decodestate(&d._state);

    int outputStart=0,outputlength = 0;
    int codelength = strlen(input);
    const char* pIn = input;
    int stepCodeLength = 0;
    do
    {
        output.resize(outputStart+N);
        stepCodeLength = codelength>=N?N:codelength;
        outputlength = d.decode(pIn, stepCodeLength, &output[outputStart]);
        outputStart+=outputlength;
        pIn+=stepCodeLength;
        codelength-=stepCodeLength;
    }
    while (codelength>0);

    output.resize(outputStart);
    //
    base64_init_decodestate(&d._state);
    return true;
}

template <typename VectorChar> static bool Base64Encode(const char* input,int inputSize,VectorChar& output)
{
	output.clear();if (!input || inputSize==0) return false;

    const int N=4096;
    base64::encoder e(N);
    base64_init_encodestate(&e._state);

    int outputStart=0,outputlength = 0;
    int codelength = inputSize;
    const char* pIn = input;
    int stepCodeLength = 0;

    do
    {
        output.resize(outputStart+2*N);
        stepCodeLength = codelength>=N?N:codelength;
        outputlength = e.encode(pIn, stepCodeLength,&output[outputStart]);
        outputStart+=outputlength;
        pIn+=stepCodeLength;
        codelength-=stepCodeLength;
    }
    while (codelength>0);

    output.resize(outputStart+2*N);
    outputlength = e.encode_end(&output[outputStart]);
    outputStart+=outputlength;
    output.resize(outputStart);
    //
    base64_init_encodestate(&e._state);

	return true;
}
inline static unsigned int Decode85Byte(char c)   { return c >= '\\' ? c-36 : c-35; }
static void Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}
template <typename VectorChar> static bool Base85Decode(const char* input,VectorChar& output)
{
	output.clear();if (!input) return false;
	const int outputSize = (((int)strlen(input) + 4) / 5) * 4;
	output.resize(outputSize);
    Decode85((const unsigned char*)input,(unsigned char*)&output[0]);
    return true;
}

inline static char Encode85Byte(unsigned int x)
{
    x = (x % 85) + 35;
    return (x>='\\') ? x+1 : x;
}
template <typename VectorChar> static bool Base85Encode(const char* input,int inputSize,VectorChar& output,bool outputStringifiedMode,int numCharsPerLineInStringifiedMode=112)	
{
    // Adapted from binary_to_compressed_c(...) inside imgui_draw.cpp
    output.clear();if (!input || inputSize==0) return false;
    output.reserve((int)((float)inputSize*1.3f));
    if (numCharsPerLineInStringifiedMode<=12) numCharsPerLineInStringifiedMode = 12;
    if (outputStringifiedMode) output.push_back('"');
    char prev_c = 0;int cnt=0;
    for (int src_i = 0; src_i < inputSize; src_i += 4)
    {
        unsigned int d = *(unsigned int*)(input + src_i);
        for (unsigned int n5 = 0; n5 < 5; n5++, d /= 85)
        {
            char c = Encode85Byte(d);
            if (outputStringifiedMode && c == '?' && prev_c == '?') output.push_back('\\');	// This is made a little more complicated by the fact that ??X sequences are interpreted as trigraphs by old C/C++ compilers. So we need to escape pairs of ??.
            output.push_back(c);
            prev_c = c;
        }
        cnt+=4;
        if (outputStringifiedMode && cnt>=numCharsPerLineInStringifiedMode)
        {
            output.push_back('"');
            output.push_back('	');
            output.push_back('\\');
            //output.push_back(' ');
            output.push_back('\n');
            output.push_back('"');
            cnt=0;
        }
    }
    // End
    if (outputStringifiedMode)
    {
        output.push_back('"');
        output.push_back(';');
        output.push_back('\n');
        output.push_back('\n');
    }
    output.push_back('\0');	// End character

    return true;
}
} // namespace Stringifier

bool Base64Encode(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode,int numCharsPerLineInStringifiedMode)
{
    if (!stringifiedMode) return Stringifier::Base64Encode<ImVector<char> >(input,inputSize,output);
    else
    {
        ImVector<char> output1;
        if (!Stringifier::Base64Encode<ImVector<char> >(input,inputSize,output1)) {output.clear();return false;}
        if (output1.size()==0) {output.clear();return false;}
        if (!ImGui::TextStringify(&output1[0],output,numCharsPerLineInStringifiedMode,output1.size()-1)) {output.clear();return false;}
    }
    return true;
}

bool Base64Decode(const char* input,ImVector<char>& output)
{
    return Stringifier::Base64Decode<ImVector<char> >(input,output);
}

bool Base85Encode(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode,int numCharsPerLineInStringifiedMode)
{
    return Stringifier::Base85Encode<ImVector<char> >(input,inputSize,output,stringifiedMode,numCharsPerLineInStringifiedMode);
}

bool Base85Decode(const char* input,ImVector<char>& output)
{
    return Stringifier::Base85Decode<ImVector<char> >(input,output);
}

bool BinaryStringify(const char* input, int inputSize, ImVector<char>& output, int numInputBytesPerLineInStringifiedMode,bool serializeUnsignedBytes)
{
    output.clear();
    if (!input || inputSize<=0) return false;
    ImGuiTextBuffer b;
    b.clear();
    b.Buf.reserve(inputSize*7.5f);
    // -----------------------------------------------------------
    if (serializeUnsignedBytes)
    {
        b.appendf("{%d",(int) (*((unsigned char*) &input[0])));  // start byte
        int cnt=1;
        for (int i=1;i<inputSize;i++)
        {
            if (cnt++>=numInputBytesPerLineInStringifiedMode) {cnt=0;b.appendf("\n");}
            b.appendf(",%d",(int) (*((unsigned char*) &input[i])));
        }
    }
    else
    {
        b.appendf("{%d",(int)input[0]);  // start byte
        int cnt=1;
        for (int i=1;i<inputSize;i++)
        {
            if (cnt++>=numInputBytesPerLineInStringifiedMode) {cnt=0;b.appendf("\n");}
            b.appendf(",%d",(int)input[i]);
        }
    }
    b.appendf("};\n");
    //-------------------------------------------------------------
    b.Buf.swap(output);
    return true;
}

bool TextStringify(const char* input, ImVector<char>& output, int numCharsPerLineInStringifiedMode, int inputSize, bool noBackslashAtLineEnds)
{
    output.clear();if (!input) return false;
    if (inputSize<=0) inputSize=strlen(input);
    output.reserve(inputSize*1.25f);
    // --------------------------------------------------------------
    output.push_back('"');
    char c='\n';int cnt=0;bool endFile = false;
    for (int i=0;i<inputSize;i++)
    {
        c = input[i];
        switch (c)
        {
        case '\\':
            output.push_back('\\');
            output.push_back('\\');
            break;
        case '"':
            output.push_back('\\');
            output.push_back('"');
            break;
        case '\r':
        case '\n':
            //---------------------
            output.push_back('\\');
            output.push_back(c=='\n' ? 'n' : 'r');
            if (numCharsPerLineInStringifiedMode<=0)
            {
                // Break at newline to ease reading:
                output.push_back('"');                
                if (i==inputSize-1)
                {
                    endFile = true;
                    if (!noBackslashAtLineEnds) output.push_back(';');
                    output.push_back('\n');
                }
                else
                {
                    if (!noBackslashAtLineEnds)
                    {
                        output.push_back('\t');
                        output.push_back('\\');
                    }
                    output.push_back('\n');
                    output.push_back('"');
                }
                cnt = 0;
                //--------------------
            }
            //--------------------
            break;
        default:
            output.push_back(c);
            if (++cnt>=numCharsPerLineInStringifiedMode && numCharsPerLineInStringifiedMode>0)
            {
                //---------------------
                //output.push_back('\\');
                //output.push_back('n');
                output.push_back('"');

                if (i==inputSize-1)
                {
                    endFile = true;
                    if (!noBackslashAtLineEnds) output.push_back(';');
                    output.push_back('\n');
                }
                else
                {
                    if (!noBackslashAtLineEnds)
                    {
                        output.push_back('\t');
                        output.push_back('\\');
                    }
                    output.push_back('\n');
                    output.push_back('"');
                }
                cnt = 0;
                //--------------------
            }
            break;
        }
    }

    if (!endFile)
    {
        output.push_back('"');
        if (!noBackslashAtLineEnds) output.push_back(';');
        output.push_back('\n');
        //--------------------
    }

    output.push_back('\0');	// End character
    //-------------------------------------------------------------
    return true;
}
} // namespace ImGui

#ifdef IMGUI_USE_ZLIB	// requires linking to library -lZlib
#include <zlib.h>
namespace ImGui
{
bool GzDecompressFromFile(const char* filePath,ImVector<char>& rv,bool clearRvBeforeUsage)
{
    if (clearRvBeforeUsage) rv.clear();
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"rb",false)) return false;
    //----------------------------------------------------
    return GzDecompressFromMemory(&f_data[0],f_data.size(),rv,clearRvBeforeUsage);
    //----------------------------------------------------
}
bool GzBase64DecompressFromFile(const char* filePath,ImVector<char>& rv)
{
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::GzBase64DecompressFromMemory(&f_data[0],rv);
}
bool GzBase85DecompressFromFile(const char* filePath,ImVector<char>& rv)
{
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::GzBase85DecompressFromMemory(&f_data[0],rv);
}

bool GzDecompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage)
{
    if (clearRvBeforeUsage) rv.clear();
    const int startRv = rv.size();

    if (memoryBufferSize == 0  || !memoryBuffer) return false;
    const int memoryChunk = memoryBufferSize > (16*1024) ? (16*1024) : memoryBufferSize;
    rv.resize(startRv+memoryChunk);  // we start using the memoryChunk length

    z_stream myZStream;
    myZStream.next_in = (Bytef *) memoryBuffer;
    myZStream.avail_in = memoryBufferSize;
    myZStream.total_out = 0;
    myZStream.zalloc = Z_NULL;
    myZStream.zfree = Z_NULL;

    bool done = false;
    if (inflateInit2(&myZStream, (16+MAX_WBITS)) == Z_OK)
    {
        int err = Z_OK;
        while (!done)
        {
            if (myZStream.total_out >= (uLong)(rv.size()-startRv)) rv.resize(rv.size()+memoryChunk);    // not enough space: we add the memoryChunk each step

            myZStream.next_out = (Bytef *) (&rv[startRv] + myZStream.total_out);
            myZStream.avail_out = rv.size() - startRv - myZStream.total_out;

            if ((err = inflate (&myZStream, Z_SYNC_FLUSH))==Z_STREAM_END) done = true;
            else if (err != Z_OK)  break;
        }
        if ((err=inflateEnd(&myZStream))!= Z_OK) done = false;
    }
    rv.resize(startRv+(done ? myZStream.total_out : 0));

    return done;
}
bool GzCompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage) 
{
    if (clearRvBeforeUsage) rv.clear();
    const int startRv = rv.size();

    if (memoryBufferSize == 0  || !memoryBuffer) return false;
    const int memoryChunk = memoryBufferSize/3 > (16*1024) ? (16*1024) : memoryBufferSize/3;
    rv.resize(startRv+memoryChunk);  // we start using the memoryChunk length

    z_stream myZStream;
    myZStream.next_in =  (Bytef *) memoryBuffer;
    myZStream.avail_in = memoryBufferSize;
    myZStream.total_out = 0;
    myZStream.zalloc = Z_NULL;
    myZStream.zfree = Z_NULL;

    bool done = false;
    if (deflateInit2(&myZStream,Z_BEST_COMPRESSION,Z_DEFLATED,(16+MAX_WBITS),8,Z_DEFAULT_STRATEGY) == Z_OK)
    {
        int err = Z_OK;
        while (!done)
        {
            if (myZStream.total_out >= (uLong)(rv.size()-startRv)) rv.resize(rv.size()+memoryChunk);    // not enough space: we add the full memoryChunk each step

            myZStream.next_out = (Bytef *) (&rv[startRv] + myZStream.total_out);
            myZStream.avail_out = rv.size() - startRv - myZStream.total_out;

            if ((err = deflate (&myZStream, Z_FINISH))==Z_STREAM_END) done = true;
            else if (err != Z_OK)  break;
        }
        if ((err=deflateEnd(&myZStream))!= Z_OK) done=false;
    }
    rv.resize(startRv+(done ? myZStream.total_out : 0));

    return done;
}

bool GzBase64DecompressFromMemory(const char* input,ImVector<char>& rv)
{
    rv.clear();ImVector<char> v;
    if (ImGui::Base64Decode(input,v)) return false;
    if (v.size()==0) return false;
    return GzDecompressFromMemory(&v[0],v.size(),rv);
}
bool GzBase85DecompressFromMemory(const char* input,ImVector<char>& rv)
{
    rv.clear();ImVector<char> v;
    if (ImGui::Base85Decode(input,v)) return false;
    if (v.size()==0) return false;
    return GzDecompressFromMemory(&v[0],v.size(),rv);
}
bool GzBase64CompressFromMemory(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode,int numCharsPerLineInStringifiedMode)
{
    output.clear();ImVector<char> output1;
    if (!ImGui::GzCompressFromMemory(input,inputSize,output1)) return false;
    return ImGui::Base64Encode(&output1[0],output1.size(),output,stringifiedMode,numCharsPerLineInStringifiedMode);
}
bool GzBase85CompressFromMemory(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode,int numCharsPerLineInStringifiedMode)
{
    output.clear();ImVector<char> output1;
    if (!ImGui::GzCompressFromMemory(input,inputSize,output1)) return false;
    return ImGui::Base85Encode(&output1[0],output1.size(),output,stringifiedMode,numCharsPerLineInStringifiedMode);
}
} // namespace ImGui
#endif //IMGUI_USE_ZLIB

namespace ImGui
{
// Two methods that fill rv and return true on success
bool Base64DecodeFromFile(const char* filePath,ImVector<char>& rv)
{
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::Base64Decode(&f_data[0],rv);
}
bool Base85DecodeFromFile(const char* filePath,ImVector<char>& rv)
{
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::Base85Decode(&f_data[0],rv);
}
} // namespace ImGui

namespace ImGui
{
// Generate color
void RandomColor(ImVec4& color, float alpha)
{
    alpha = ImClamp(alpha, 0.0f, 1.0f);
    int r = std::rand() % 255;
    int g = std::rand() % 255;
    int b = std::rand() % 255;
    color = ImVec4(r / 255.0, g / 255.0, b / 255.0, alpha);
}

void RandomColor(ImU32& color, float alpha)
{
    alpha = ImClamp(alpha, 0.0f, 1.0f);
    int r = std::rand() % 255;
    int g = std::rand() % 255;
    int b = std::rand() % 255;
    color = IM_COL32(r, g, b, (int)(alpha * 255.f));
}
} // namespace ImGui

namespace ImGui
{
// FFT 1D
inline void swap(float* a, float* b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

inline float sqr(float arg)
{
	return arg * arg;
}

void ImFFT(float* data, int N,  bool forward)
{
	int n = N << 1;
	int i, j, m, mmax;

	/* bit reversal section */

	j = 0;
	for (i = 0; i < n; i += 2) 
	{
		if (j > i) 
		{
			swap (&data[j], &data[i]);
			swap (&data[j + 1], &data[i + 1]);
		}
		m = N;
		while (m >= 2 && j >= m) 
		{
			j -= m;
			m >>= 1;
		}
		j += m;
	}

	/* Daniel-Lanczos section */

	float theta, wr, wpr, wpi, wi, wtemp;
	float tempr, tempi;
	for (mmax = 2; n > mmax;) 
	{
		int istep = mmax << 1;
		theta = (forward ? 1 : -1) * (2.0 * M_PI / mmax);
		wpr = -2.0 * sqr (sin (0.5 * theta));
		wpi = sin (theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 0; m < mmax; m += 2) 
		{
			for (i = m; i < n; i += istep) 
			{
				j = i + mmax;
				tempr = wr * data[j] - wi * data[j + 1];
				tempi = wr * data[j + 1] + wi * data[j];
				data[j] = data[i] - tempr;
				data[j + 1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i + 1] += tempi;
			}
			wr = (wtemp = wr) * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp * wpi + wi;
		}
		mmax = istep;
	}

	/* normalisation section */
	const float tmp = 1.0 / sqrt ((float)N);
	for (i = 0; i < n; i++)
		data[i] *= tmp;
}

void ImRFFT(float* data, int N,  bool forward)
{
	/* main section */
	int k;
	float c1 = 0.5, c2;
	float theta = M_PI / (float) (N >> 1);

	if (forward) 
	{
		c2 = -0.5;
		ImFFT(data, N >> 1, forward);
	} 
	else 
	{
		c2 = +0.5;
		theta = -theta;
	}

	float wpr = -2.0 * sqr (sin (0.5 * theta));
	float wpi = sin (theta);
	float wr = 1.0 + wpr;
	float wi = wpi;
	float wtemp;

	int i, i1, i2, i3, i4;
	float h1r, h1i, h2r, h2i;
	for (i = 1; i < N >> 2; i++) 
	{
		i1 = i + i;
		i2 = i1 + 1;
		i3 = N - i1;
		i4 = i3 + 1;
		h1r = c1 * (data[i1] + data[i3]);
		h1i = c1 * (data[i2] - data[i4]);
		h2r = -c2 * (data[i2] + data[i4]);
		h2i = c2 * (data[i1] - data[i3]);
		data[i1] = h1r + wr * h2r - wi * h2i;
		data[i2] = h1i + wr * h2i + wi * h2r;
		data[i3] = h1r - wr * h2r + wi * h2i;
		data[i4] = -h1i + wr * h2i + wi * h2r;
		wr = (wtemp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	}

	if (forward) 
	{
		data[0] = (h1r = data[0]) + data[1];
		data[1] = h1r - data[1];
	}
	else 
	{
		data[0] = c1 * ((h1r = data[0]) + data[1]);
		data[1] = c1 * (h1r - data[1]);
		ImFFT(data, N >> 1, forward);
	}

	/* normalisation section */
	//const float tmp = forward ? M_SQRT1_2 : M_SQRT2;
	//for (k = 0; k < N; k++)
	//	data[k] *= tmp;
}

void ImRFFT(float* in, float* out, int N,  bool forward)
{
    memcpy(out, in, sizeof(float) * N);
    ImRFFT(out, N, forward);
}

int ImReComposeDB(float * in, float * out, int samples, bool inverse)
{
	int i,max = 0;
	float zero_db,db,max_db = -FLT_MAX;
	float amplitude;
    int N = samples >> 1;
	zero_db = - 20 * log10((float)(1<<15));
	for (i = 0; i < N + 1; i++)
	{
		if (i != 0/* && i != N*/)
		{
			amplitude = sqrt(sqr(in[2 * i]) + sqr(in[2 * i + 1]));
		}
		else
		{
			amplitude = 1.0f / (1<<15);
		}
		db = 20 * log10(amplitude) - (inverse ? zero_db : 0);
		out[i] = db;
		if (db > max_db)
		{
			max_db = db;
			max = i;
		}
	}
	return max;
}

int ImReComposeAmplitude(float * in, float * out, int samples)
{
	int i,max = 0;
	float tmp,dmax = 0;
	for (i = 0; i < (samples >> 1) + 1; i++)
	{
		if (i != 0/* && i != (samples >> 1)*/)
		{
			tmp = sqrt(sqr(in[2 * i]) + sqr(in[2 * i + 1]));
		}
		else
		{
			tmp = 0;//sqr(in[i == 0 ? 0 : 1]);
		}
		out[i] = tmp;
		if (tmp > dmax)
		{
			dmax = tmp;
			max = i;
		}
	}
	return max;
}

int ImReComposePhase(float * in, float * out, int samples)
{
    for (int i = 0; i < (samples >> 1) + 1; i++)
	{
        float hAngle = 0;
        float dx = in[2 * i];
        float dy = in[2 * i + 1];
        hAngle = atan2(dy, dx);
        hAngle = 180.f * hAngle / M_PI;
        out[i] = hAngle;
    }
	return 0;
}

int ImReComposeDBShort(float * in, float * out, int samples, bool inverse)
{
	int i,j;
	int n_sample;
	int start_sample;
	float zero_db;
	float tmp;
	static unsigned int freq_table[21] = {0,1,2,3,4,5,6,7,8,11,15,20,
		27,36,47,62,82,107,141,184,255}; //fft_number

	zero_db = - 20 * log10((float)(1<<15));

	for (i = 0; i< 20; i++)
	{
		start_sample = freq_table[i] * (samples / 256);
		n_sample = (freq_table[i + 1] - freq_table[i])  * (samples / 256);
		tmp=0;
		for (j = start_sample; j < start_sample + n_sample; j++)
		{
			tmp += 2 * sqr(in[j]);
		}
		
		tmp /= (float)n_sample;
		out[i] = 20.0 * log10(tmp) - (inverse ? zero_db : 0);
	}
	return 20;
}

int ImReComposeDBLong(float * in, float * out, int samples, bool inverse)
{
	int i,j;
	int n_sample;
	int start_sample;
	float zero_db;
	float tmp;
	static unsigned int freq_table[77] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,
		19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,
		35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
		52,53,54,55,56,57,58,61,66,71,76,81,87,93,100,107,
		114,122,131,140,150,161,172,184,255}; //fft_number

	zero_db = - 20 * log10((float)(1<<15));

	for (i = 0; i< 76; i++)
	{
		start_sample = freq_table[i] * (samples / 256);
		n_sample = (freq_table[i + 1] - freq_table[i]) * (samples / 256);
		tmp=0;
		for (j = start_sample; j < start_sample + n_sample; j++)
		{
			tmp += 2 * sqr(in[j]);
		}
		
		tmp /= (float)n_sample;
		out[i] = 20.0 * log10(tmp) - (inverse ? zero_db : 0);
	}
	return 76;
}

float ImDoDecibel(float * in, int samples, bool inverse)
{
	int i;
	float db,zero_db;
	float tmp;
	zero_db = - 20 * log10((float)(1<<15));
	tmp = 0;
	for (i = 0; i < (samples >> 1) + 1; i++)
	{
		if (i != 0 && i != (samples >> 1))
		{
			tmp += 2 * (sqr(in[2 * i]) + sqr(in[2 * i+1]));
		}
		else
		{
			tmp += sqr(in[i == 0 ? 0 : 1]);
		}
	}
	tmp /= (float)samples;
	db = 20 * log10(tmp) - (inverse ? zero_db : 0);
	return db;
}

struct HannWindow
{
    inline HannWindow(int _frame_size, int _shift_size)
    {
        float tmp = 0;
        shift_size = _shift_size;
        frame_size = _frame_size;
        hann = new float[frame_size];
        for (int i = 0; i < frame_size; i++) hann[i] = 0.5 * (1.0 - cos(2.0 * M_PI * (i / (float)frame_size)));
        for (int i = 0; i < frame_size; i++) tmp += hann[i] * hann[i];
        tmp /= shift_size;
        tmp = std::sqrt(tmp);
        for (int i = 0; i < frame_size; i++) hann[i] /= tmp;
    }
    inline ~HannWindow() { delete[] hann; };
    inline void Process(float * buffer) { for (int i = 0; i < frame_size; i++) buffer[i] *= hann[i]; }

private:
    float *hann {nullptr};
    int shift_size;
    int frame_size;
};

struct WindowOverlap
{
public:
    inline WindowOverlap(uint32_t _frame_size, uint32_t _shift_size)
    {
        frame_size = _frame_size;
        shift_size = _shift_size;
        buf_offset = 0;
        num_block = frame_size / shift_size;
        output = new float[shift_size];
        buf = new float[frame_size];
        memset(buf, 0, frame_size * sizeof(float));
    }
    inline ~WindowOverlap() { delete[] buf; delete[] output; };
    inline float *overlap(float *in)
    {
        // Shift
        for (int i = 0; i < static_cast<int>(frame_size - shift_size); i++) buf[i] = buf[i + shift_size];
        // Emptying Last Block
        memset(buf + shift_size * (num_block - 1), 0, sizeof(float) * shift_size);
        // Sum
        for (int i = 0; i < static_cast<int>(frame_size); i++) buf[i] += in[i];
        // Distribution for float format
        for (int i = 0; i < static_cast<int>(shift_size); i++) output[i] = static_cast<float>(buf[i]);
        return output;
    }

private:
    uint32_t frame_size;
    uint32_t shift_size;
    uint32_t num_block;
    uint32_t buf_offset;
    float *output;
    float *buf;
};

ImSTFT::ImSTFT(int frame_, int shift_)
{
    frame_size = frame_;
    shift_size = shift_;
    overlap_size = frame_size - shift_size;
    hannwin = new HannWindow(frame_size, shift_size);
    overlap = new WindowOverlap(frame_size, shift_size);
    buf = new float[frame_size];
    memset(buf, 0, sizeof(float) * frame_size);
}

ImSTFT::~ImSTFT()
{ 
    delete (HannWindow*)hannwin; 
    delete (WindowOverlap*)overlap; 
    delete[] buf;
};

void ImSTFT::stft(float* in, float* out)
{
    /*** Shfit & Copy***/
    for (int i = 0; i < overlap_size; i++) buf[i] = buf[i + shift_size];
    for (int i = 0; i < shift_size; i++) buf[overlap_size + i] = static_cast<float>(in[i]);
    memcpy(out, buf, sizeof(float) * frame_size);
    /*** Window ***/
    ((HannWindow*)hannwin)->Process(out);
    /*** FFT ***/
    ImGui::ImRFFT(out, frame_size, true);
}

void ImSTFT::istft(float* in, float* out)
{
    /*** iFFT ***/
    ImGui::ImRFFT(in, frame_size, false);
    /*** Window ***/
    ((HannWindow*)hannwin)->Process(in);
    /*** Output ***/
    memcpy(out, ((WindowOverlap*)overlap)->overlap(in), sizeof(float) * shift_size);
}
} // namespace ImGui

// Kalman class
ImGui::ImKalman::ImKalman(int state_size,int mea_size)
{
    transitionMatrix.create_type(state_size, state_size, IM_DT_FLOAT32);
    measurementMatrix.create_type(state_size, mea_size, IM_DT_FLOAT32);
    processNoiseCov.create_type(state_size, state_size, IM_DT_FLOAT32);
    measurementNoiseCov.create_type(mea_size, mea_size, IM_DT_FLOAT32);
    errorCovPre.create_type(state_size, state_size, IM_DT_FLOAT32);
    errorCovPost.create_type(state_size, state_size, IM_DT_FLOAT32);
    statePost.create_type(1, state_size, IM_DT_FLOAT32);
    statePre.create_type(1, state_size, IM_DT_FLOAT32);
    K.create_type(mea_size, state_size, IM_DT_FLOAT32);

    measurementMatrix.eye(1.f);     // 观测矩阵的初始化
    processNoiseCov.eye(1e-5);      // 模型本身噪声协方差矩阵初始化
    measurementNoiseCov.eye(1e-1);  // 测量噪声的协方差矩阵初始化
    errorCovPost.eye(1.f);          // 转移噪声修正矩阵初始化
    statePost.randn(0.f, 5.0f);     // kalaman状态估计修正矩阵初始化
    transitionMatrix.eye(1.f);      // 状态转移矩阵/增益矩阵的初始化
    for (int x = 0; x < state_size; x++)
    {
        for (int y = 0; y < state_size; y++)
        {
            if (x > y && (x - state_size / 2 == y || y + state_size / 2 == x))
                transitionMatrix.at<float>(x, y) = 1.f;
        }
    }
}

void ImGui::ImKalman::covariance(float noise_covariance, float measurement_noise_covariance)
{
    processNoiseCov.eye(noise_covariance);
    measurementNoiseCov.eye(measurement_noise_covariance);
}

ImGui::ImMat& ImGui::ImKalman::predicted()
{
    statePre    = transitionMatrix * statePost;
    errorCovPre = transitionMatrix * errorCovPost * transitionMatrix.t() + processNoiseCov;
    return statePost;
}

void ImGui::ImKalman::update(ImMat& Y)
{
    K            = errorCovPre * measurementMatrix.t() * ((measurementMatrix * errorCovPre * measurementMatrix.t() + measurementNoiseCov).inv<float>());
    statePost    = statePre    + K * (Y - measurementMatrix * statePre);
    errorCovPost = errorCovPre - K * measurementMatrix * errorCovPre;
}

// warp Affine help
static inline int LU(float* A, size_t astep, int m, float* b, size_t bstep, int n, float eps)
{
    int i, j, k, p = 1;
    for( i = 0; i < m; i++ )
    {
        k = i;
        for( j = i+1; j < m; j++ ) if( std::abs(A[j*astep + i]) > std::abs(A[k*astep + i]) ) k = j;
        if( std::abs(A[k*astep + i]) < eps ) return 0;
        if( k != i )
        {
            for( j = i; j < m; j++ ) std::swap(A[i*astep + j], A[k*astep + j]);
            if( b ) for( j = 0; j < n; j++ ) std::swap(b[i*bstep + j], b[k*bstep + j]);
            p = -p;
        }
        float d = -1/A[i*astep + i];
        for( j = i+1; j < m; j++ )
        {
            float alpha = A[j*astep + i]*d;
            for( k = i+1; k < m; k++ ) A[j*astep + k] += alpha*A[i*astep + k];
            if( b ) for( k = 0; k < n; k++ ) b[j*bstep + k] += alpha*b[i*bstep + k];
        }
    }
    if( b )
    {
        for( i = m-1; i >= 0; i-- )
        {
            for( j = 0; j < n; j++ )
            {
                float s = b[i*bstep + j];
                for( k = i+1; k < m; k++ ) s -= A[i*astep + k]*b[k*bstep + j];
                b[i*bstep + j] = s/A[i*astep + i];
            }
        }
    }
    return p;
}

static bool solve(const ImGui::ImMat& src, const ImGui::ImMat& src2, ImGui::ImMat& dst)
{
    // Gaussian elimination with the optimal pivot element chosen.
    bool result = true;
    IM_ASSERT(src.type == src2.type);
    int m = src.h, m_ = m, n = src.w, nb = src2.w;
    IM_ASSERT(m >= n); // The function can not solve under-determined linear systems
    dst.clone_from(src2);
    result = LU((float*)src.data, src.w, n, (float*)dst.data, dst.w, nb, FLT_EPSILON * 10) != 0;
    return result;
}

ImGui::ImMat ImGui::getPerspectiveTransform(const ImVec2 src[], const ImVec2 dst[])
{
    float a[8][8], b[8];
    for (int i = 0; i < 4; ++i)
    {
        a[i][0] = a[i + 4][3] = src[i].x;
        a[i][1] = a[i + 4][4] = src[i].y;
        a[i][2] = a[i + 4][5] = 1;
        a[i][3] = a[i][4] = a[i][5] =
        a[i + 4][0] = a[i + 4][1] = a[i + 4][2] = 0;
        a[i][6] = -src[i].x * dst[i].x;
        a[i][7] = -src[i].y * dst[i].x;
        a[i + 4][6] = -src[i].x * dst[i].y;
        a[i + 4][7] = -src[i].y * dst[i].y;
        b[i] = dst[i].x;
        b[i + 4] = dst[i].y;
    }
    ImGui::ImMat A, B;
    A.create_type(8, 8, a, IM_DT_FLOAT32);
    B.create_type(1, 8, b, IM_DT_FLOAT32);
    ImGui::ImMat M, X;
    M.create_type(3, 3, IM_DT_FLOAT32);
    solve(A, B, X);
    memcpy(M.data, X.data, sizeof(float) * X.total());
    M.at<float>(2, 2) = 1.f;
    return M;
}

ImGui::ImMat ImGui::getAffineTransform(const ImVec2 src[], const ImVec2 dst[])
{
    float a[6*6], b[6];
    for( int i = 0; i < 3; i++ )
    {
        int j = i * 12;
        int k = i * 12 + 6;
        a[j] = a[k + 3] = src[i].x;
        a[j + 1] = a[k + 4] = src[i].y;
        a[j + 2] = a[k + 5] = 1;
        a[j + 3] = a[j + 4] = a[j + 5] = 0;
        a[k] = a[k + 1] = a[k + 2] = 0;
        b[i * 2] = dst[i].x;
        b[i * 2 + 1] = dst[i].y;
    }
    ImGui::ImMat A, B;
    A.create_type(6, 6, a, IM_DT_FLOAT32);
    B.create_type(1, 6, b, IM_DT_FLOAT32);
    ImGui::ImMat M, X;
    M.create_type(3, 2, IM_DT_FLOAT32);
    solve(A, B, X);
    memcpy(M.data, X.data, sizeof(float) * X.total());
    return M;
}

static void resize_bilinear_c1(const unsigned char* src, int srcw, int srch, int srcstride, unsigned char* dst, int w, int h, int stride)
{
    const int INTER_RESIZE_COEF_BITS = 11;
    const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;
    //     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = new int[w + h + w + h];

    int* xofs = buf;     //new int[w];
    int* yofs = buf + w; //new int[h];

    short* ialpha = (short*)(buf + w + h);    //new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w); //new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)::std::min(::std::max((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = static_cast<int>(floor(fx));
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 = fx * INTER_RESIZE_COEF_SCALE;

        ialpha[dx * 2] = SATURATE_CAST_SHORT(a0);
        ialpha[dx * 2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = static_cast<int>(floor(fy));
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 = fy * INTER_RESIZE_COEF_SCALE;

        ibeta[dy * 2] = SATURATE_CAST_SHORT(b0);
        ibeta[dy * 2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    ImGui::ImMat rowsbuf0(w, (size_t)2u);
    ImGui::ImMat rowsbuf1(w, (size_t)2u);
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++)
    {
        sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 1)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S1p = S1 + sx;
                rows1p[dx] = (S1p[0] * a0 + S1p[1] * a1) >> 4;

                ialphap += 2;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char* S0 = src + srcstride * (sy);
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;
                rows0p[dx] = (S0p[0] * a0 + S0p[1] * a1) >> 4;
                rows1p[dx] = (S1p[0] * a0 + S1p[1] * a1) >> 4;

                ialphap += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + stride * (dy);

#if __ARM_NEON || __SSE__ || __AVX__
        int nn = w >> 3;
#else
        int nn = 0;
#endif
        int remain = w - (nn << 3);

#if __ARM_NEON || __SSE__ || __AVX__
#if __aarch64__ || __SSE__ || __AVX__
        int16x4_t _b0 = vdup_n_s16(b0);
        int16x4_t _b1 = vdup_n_s16(b1);
        int32x4_t _v2 = vdupq_n_s32(2);
        for (; nn > 0; nn--)
        {
            int16x4_t _rows0p_sr4 = vld1_s16(rows0p);
            int16x4_t _rows1p_sr4 = vld1_s16(rows1p);
            int16x4_t _rows0p_1_sr4 = vld1_s16(rows0p + 4);
            int16x4_t _rows1p_1_sr4 = vld1_s16(rows1p + 4);

            int32x4_t _rows0p_sr4_mb0 = vmull_s16(_rows0p_sr4, _b0);
            int32x4_t _rows1p_sr4_mb1 = vmull_s16(_rows1p_sr4, _b1);
            int32x4_t _rows0p_1_sr4_mb0 = vmull_s16(_rows0p_1_sr4, _b0);
            int32x4_t _rows1p_1_sr4_mb1 = vmull_s16(_rows1p_1_sr4, _b1);

            int32x4_t _acc = _v2;
            _acc = vsraq_n_s32(_acc, _rows0p_sr4_mb0, 16);
            _acc = vsraq_n_s32(_acc, _rows1p_sr4_mb1, 16);

            int32x4_t _acc_1 = _v2;
            _acc_1 = vsraq_n_s32(_acc_1, _rows0p_1_sr4_mb0, 16);
            _acc_1 = vsraq_n_s32(_acc_1, _rows1p_1_sr4_mb1, 16);

            int16x4_t _acc16 = vshrn_n_s32(_acc, 2);
            int16x4_t _acc16_1 = vshrn_n_s32(_acc_1, 2);

            uint8x8_t _D = vqmovun_s16(vcombine_s16(_acc16, _acc16_1));

            vst1_u8(Dp, _D);

            Dp += 8;
            rows0p += 8;
            rows1p += 8;
        }
#else
        if (nn > 0)
        {
            asm volatile(
                "vdup.s16   d16, %8         \n"
                "mov        r4, #2          \n"
                "vdup.s16   d17, %9         \n"
                "vdup.s32   q12, r4         \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "0:                         \n"
                "vmull.s16  q0, d2, d16     \n"
                "vmull.s16  q1, d3, d16     \n"
                "vorr.s32   q10, q12, q12   \n"
                "vorr.s32   q11, q12, q12   \n"
                "vmull.s16  q2, d6, d17     \n"
                "vmull.s16  q3, d7, d17     \n"
                "vsra.s32   q10, q0, #16    \n"
                "vsra.s32   q11, q1, #16    \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "vsra.s32   q10, q2, #16    \n"
                "vsra.s32   q11, q3, #16    \n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "vshrn.s32  d20, q10, #2    \n"
                "vshrn.s32  d21, q11, #2    \n"
                "vqmovun.s16 d20, q10        \n"
                "vst1.8     {d20}, [%2]!    \n"
                "subs       %3, #1          \n"
                "bne        0b              \n"
                "sub        %0, #16         \n"
                "sub        %1, #16         \n"
                : "=r"(rows0p), // %0
                "=r"(rows1p), // %1
                "=r"(Dp),     // %2
                "=r"(nn)      // %3
                : "0"(rows0p),
                "1"(rows1p),
                "2"(Dp),
                "3"(nn),
                "r"(b0), // %8
                "r"(b1)  // %9
                : "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12");
        }
#endif // __aarch64__ || __SSE__ || __AVX__
#endif // __ARM_NEON || __SSE__ || __AVX__
        for (; remain; --remain)
        {
            //             D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(((short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >> 2);
        }

        ibeta += 2;
    }

    delete[] buf;
}

static void resize_bilinear_c2(const unsigned char* src, int srcw, int srch, int srcstride, unsigned char* dst, int w, int h, int stride)
{
    const int INTER_RESIZE_COEF_BITS = 11;
    const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;
    //     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = new int[w + h + w + h];

    int* xofs = buf;     //new int[w];
    int* yofs = buf + w; //new int[h];

    short* ialpha = (short*)(buf + w + h);    //new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w); //new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)::std::min(::std::max((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = static_cast<int>(floor(fx));
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx * 2;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 = fx * INTER_RESIZE_COEF_SCALE;

        ialpha[dx * 2] = SATURATE_CAST_SHORT(a0);
        ialpha[dx * 2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = static_cast<int>(floor(fy));
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 = fy * INTER_RESIZE_COEF_SCALE;

        ibeta[dy * 2] = SATURATE_CAST_SHORT(b0);
        ibeta[dy * 2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    ImGui::ImMat rowsbuf0(w * 2 + 2, (size_t)2u);
    ImGui::ImMat rowsbuf1(w * 2 + 2, (size_t)2u);
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++)
    {
        sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 1)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];

                const unsigned char* S1p = S1 + sx;
#if __ARM_NEON || __SSE__ || __AVX__
                int16x4_t _a0a1XX = vld1_s16(ialphap);
                int16x4_t _a0a0a1a1 = vzip_s16(_a0a1XX, _a0a1XX).val[0];
                uint8x8_t _S1 = uint8x8_t();

                _S1 = vld1_lane_u8(S1p, _S1, 0);
                _S1 = vld1_lane_u8(S1p + 1, _S1, 1);
                _S1 = vld1_lane_u8(S1p + 2, _S1, 2);
                _S1 = vld1_lane_u8(S1p + 3, _S1, 3);

                int16x8_t _S116 = vreinterpretq_s16_u16(vmovl_u8(_S1));
                int16x4_t _S1lowhigh = vget_low_s16(_S116);
                int32x4_t _S1ma0a1 = vmull_s16(_S1lowhigh, _a0a0a1a1);
                int32x2_t _rows1low = vadd_s32(vget_low_s32(_S1ma0a1), vget_high_s32(_S1ma0a1));
                int32x4_t _rows1 = vcombine_s32(_rows1low, vget_high_s32(_S1ma0a1));
                int16x4_t _rows1_sr4 = vshrn_n_s32(_rows1, 4);
                vst1_s16(rows1p, _rows1_sr4);
#else
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                rows1p[0] = (S1p[0] * a0 + S1p[2] * a1) >> 4;
                rows1p[1] = (S1p[1] * a0 + S1p[3] * a1) >> 4;
#endif // __ARM_NEON || __SSE__ || __AVX__

                ialphap += 2;
                rows1p += 2;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char* S0 = src + srcstride * (sy);
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;
#if __ARM_NEON || __SSE__ || __AVX__
                int16x4_t _a0 = vdup_n_s16(a0);
                int16x4_t _a1 = vdup_n_s16(a1);
                uint8x8_t _S0 = uint8x8_t();
                uint8x8_t _S1 = uint8x8_t();

                _S0 = vld1_lane_u8(S0p, _S0, 0);
                _S0 = vld1_lane_u8(S0p + 1, _S0, 1);
                _S0 = vld1_lane_u8(S0p + 2, _S0, 2);
                _S0 = vld1_lane_u8(S0p + 3, _S0, 3);

                _S1 = vld1_lane_u8(S1p, _S1, 0);
                _S1 = vld1_lane_u8(S1p + 1, _S1, 1);
                _S1 = vld1_lane_u8(S1p + 2, _S1, 2);
                _S1 = vld1_lane_u8(S1p + 3, _S1, 3);

                int16x8_t _S016 = vreinterpretq_s16_u16(vmovl_u8(_S0));
                int16x8_t _S116 = vreinterpretq_s16_u16(vmovl_u8(_S1));
                int16x4_t _S0lowhigh = vget_low_s16(_S016);
                int16x4_t _S1lowhigh = vget_low_s16(_S116);
                int32x2x2_t _S0S1low_S0S1high = vtrn_s32(vreinterpret_s32_s16(_S0lowhigh), vreinterpret_s32_s16(_S1lowhigh));
                int32x4_t _rows01 = vmull_s16(vreinterpret_s16_s32(_S0S1low_S0S1high.val[0]), _a0);
                _rows01 = vmlal_s16(_rows01, vreinterpret_s16_s32(_S0S1low_S0S1high.val[1]), _a1);
                int16x4_t _rows01_sr4 = vshrn_n_s32(_rows01, 4);
                int16x4_t _rows1_sr4 = vext_s16(_rows01_sr4, _rows01_sr4, 2);
                vst1_s16(rows0p, _rows01_sr4);
                vst1_s16(rows1p, _rows1_sr4);
#else
                rows0p[0] = (S0p[0] * a0 + S0p[2] * a1) >> 4;
                rows0p[1] = (S0p[1] * a0 + S0p[3] * a1) >> 4;
                rows1p[0] = (S1p[0] * a0 + S1p[2] * a1) >> 4;
                rows1p[1] = (S1p[1] * a0 + S1p[3] * a1) >> 4;
#endif // __ARM_NEON || __SSE__ || __AVX__

                ialphap += 2;
                rows0p += 2;
                rows1p += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + stride * (dy);

#if __ARM_NEON || __SSE__ || __AVX__
        int nn = (w * 2) >> 3;
#else
        int nn = 0;
#endif
        int remain = (w * 2) - (nn << 3);

#if __ARM_NEON || __SSE__ || __AVX__
#if __aarch64__ || __SSE__ || __AVX__
        int16x4_t _b0 = vdup_n_s16(b0);
        int16x4_t _b1 = vdup_n_s16(b1);
        int32x4_t _v2 = vdupq_n_s32(2);
        for (; nn > 0; nn--)
        {
            int16x4_t _rows0p_sr4 = vld1_s16(rows0p);
            int16x4_t _rows1p_sr4 = vld1_s16(rows1p);
            int16x4_t _rows0p_1_sr4 = vld1_s16(rows0p + 4);
            int16x4_t _rows1p_1_sr4 = vld1_s16(rows1p + 4);

            int32x4_t _rows0p_sr4_mb0 = vmull_s16(_rows0p_sr4, _b0);
            int32x4_t _rows1p_sr4_mb1 = vmull_s16(_rows1p_sr4, _b1);
            int32x4_t _rows0p_1_sr4_mb0 = vmull_s16(_rows0p_1_sr4, _b0);
            int32x4_t _rows1p_1_sr4_mb1 = vmull_s16(_rows1p_1_sr4, _b1);

            int32x4_t _acc = _v2;
            _acc = vsraq_n_s32(_acc, _rows0p_sr4_mb0, 16);
            _acc = vsraq_n_s32(_acc, _rows1p_sr4_mb1, 16);

            int32x4_t _acc_1 = _v2;
            _acc_1 = vsraq_n_s32(_acc_1, _rows0p_1_sr4_mb0, 16);
            _acc_1 = vsraq_n_s32(_acc_1, _rows1p_1_sr4_mb1, 16);

            int16x4_t _acc16 = vshrn_n_s32(_acc, 2);
            int16x4_t _acc16_1 = vshrn_n_s32(_acc_1, 2);

            uint8x8_t _D = vqmovun_s16(vcombine_s16(_acc16, _acc16_1));

            vst1_u8(Dp, _D);

            Dp += 8;
            rows0p += 8;
            rows1p += 8;
        }
#else
        if (nn > 0)
        {
            asm volatile(
                "vdup.s16   d16, %8         \n"
                "mov        r4, #2          \n"
                "vdup.s16   d17, %9         \n"
                "vdup.s32   q12, r4         \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "0:                         \n"
                "vmull.s16  q0, d2, d16     \n"
                "vmull.s16  q1, d3, d16     \n"
                "vorr.s32   q10, q12, q12   \n"
                "vorr.s32   q11, q12, q12   \n"
                "vmull.s16  q2, d6, d17     \n"
                "vmull.s16  q3, d7, d17     \n"
                "vsra.s32   q10, q0, #16    \n"
                "vsra.s32   q11, q1, #16    \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "vsra.s32   q10, q2, #16    \n"
                "vsra.s32   q11, q3, #16    \n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "vshrn.s32  d20, q10, #2    \n"
                "vshrn.s32  d21, q11, #2    \n"
                "vqmovun.s16 d20, q10        \n"
                "vst1.8     {d20}, [%2]!    \n"
                "subs       %3, #1          \n"
                "bne        0b              \n"
                "sub        %0, #16         \n"
                "sub        %1, #16         \n"
                : "=r"(rows0p), // %0
                "=r"(rows1p), // %1
                "=r"(Dp),     // %2
                "=r"(nn)      // %3
                : "0"(rows0p),
                "1"(rows1p),
                "2"(Dp),
                "3"(nn),
                "r"(b0), // %8
                "r"(b1)  // %9
                : "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12");
        }
#endif // __aarch64__ || __SSE__ || __AVX__
#endif // __ARM_NEON || __SSE__ || __AVX__
        for (; remain; --remain)
        {
            //             D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(((short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >> 2);
        }

        ibeta += 2;
    }

    delete[] buf;
}

static void resize_bilinear_c3(const unsigned char* src, int srcw, int srch, int srcstride, unsigned char* dst, int w, int h, int stride)
{
    const int INTER_RESIZE_COEF_BITS = 11;
    const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;
    //     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = new int[w + h + w + h];

    int* xofs = buf;     //new int[w];
    int* yofs = buf + w; //new int[h];

    short* ialpha = (short*)(buf + w + h);    //new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w); //new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)::std::min(::std::max((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = static_cast<int>(floor(fx));
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx * 3;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 = fx * INTER_RESIZE_COEF_SCALE;

        ialpha[dx * 2] = SATURATE_CAST_SHORT(a0);
        ialpha[dx * 2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = static_cast<int>(floor(fy));
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 = fy * INTER_RESIZE_COEF_SCALE;

        ibeta[dy * 2] = SATURATE_CAST_SHORT(b0);
        ibeta[dy * 2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    ImGui::ImMat rowsbuf0(w * 3 + 1, (size_t)2u);
    ImGui::ImMat rowsbuf1(w * 3 + 1, (size_t)2u);
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++)
    {
        sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 1)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S1p = S1 + sx;
#if __ARM_NEON || __SSE__ || __AVX__
                int16x4_t _a0 = vdup_n_s16(a0);
                int16x4_t _a1 = vdup_n_s16(a1);
                uint8x8_t _S1 = uint8x8_t();

                _S1 = vld1_lane_u8(S1p, _S1, 0);
                _S1 = vld1_lane_u8(S1p + 1, _S1, 1);
                _S1 = vld1_lane_u8(S1p + 2, _S1, 2);
                _S1 = vld1_lane_u8(S1p + 3, _S1, 3);
                _S1 = vld1_lane_u8(S1p + 4, _S1, 4);
                _S1 = vld1_lane_u8(S1p + 5, _S1, 5);

                int16x8_t _S116 = vreinterpretq_s16_u16(vmovl_u8(_S1));
                int16x4_t _S1low = vget_low_s16(_S116);
                int16x4_t _S1high = vext_s16(_S1low, vget_high_s16(_S116), 3);
                int32x4_t _rows1 = vmull_s16(_S1low, _a0);
                _rows1 = vmlal_s16(_rows1, _S1high, _a1);
                int16x4_t _rows1_sr4 = vshrn_n_s32(_rows1, 4);
                vst1_s16(rows1p, _rows1_sr4);
#else
                rows1p[0] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1p[1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1p[2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
#endif // __ARM_NEON || __SSE__ || __AVX__

                ialphap += 2;
                rows1p += 3;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char* S0 = src + srcstride * (sy);
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;
#if __ARM_NEON || __SSE__ || __AVX__
                int16x4_t _a0 = vdup_n_s16(a0);
                int16x4_t _a1 = vdup_n_s16(a1);
                uint8x8_t _S0 = uint8x8_t();
                uint8x8_t _S1 = uint8x8_t();

                _S0 = vld1_lane_u8(S0p, _S0, 0);
                _S0 = vld1_lane_u8(S0p + 1, _S0, 1);
                _S0 = vld1_lane_u8(S0p + 2, _S0, 2);
                _S0 = vld1_lane_u8(S0p + 3, _S0, 3);
                _S0 = vld1_lane_u8(S0p + 4, _S0, 4);
                _S0 = vld1_lane_u8(S0p + 5, _S0, 5);

                _S1 = vld1_lane_u8(S1p, _S1, 0);
                _S1 = vld1_lane_u8(S1p + 1, _S1, 1);
                _S1 = vld1_lane_u8(S1p + 2, _S1, 2);
                _S1 = vld1_lane_u8(S1p + 3, _S1, 3);
                _S1 = vld1_lane_u8(S1p + 4, _S1, 4);
                _S1 = vld1_lane_u8(S1p + 5, _S1, 5);

                int16x8_t _S016 = vreinterpretq_s16_u16(vmovl_u8(_S0));
                int16x8_t _S116 = vreinterpretq_s16_u16(vmovl_u8(_S1));
                int16x4_t _S0low = vget_low_s16(_S016);
                int16x4_t _S1low = vget_low_s16(_S116);
                int16x4_t _S0high = vext_s16(_S0low, vget_high_s16(_S016), 3);
                int16x4_t _S1high = vext_s16(_S1low, vget_high_s16(_S116), 3);
                int32x4_t _rows0 = vmull_s16(_S0low, _a0);
                int32x4_t _rows1 = vmull_s16(_S1low, _a0);
                _rows0 = vmlal_s16(_rows0, _S0high, _a1);
                _rows1 = vmlal_s16(_rows1, _S1high, _a1);
                int16x4_t _rows0_sr4 = vshrn_n_s32(_rows0, 4);
                int16x4_t _rows1_sr4 = vshrn_n_s32(_rows1, 4);
                vst1_s16(rows0p, _rows0_sr4);
                vst1_s16(rows1p, _rows1_sr4);
#else
                rows0p[0] = (S0p[0] * a0 + S0p[3] * a1) >> 4;
                rows0p[1] = (S0p[1] * a0 + S0p[4] * a1) >> 4;
                rows0p[2] = (S0p[2] * a0 + S0p[5] * a1) >> 4;
                rows1p[0] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1p[1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1p[2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
#endif // __ARM_NEON || __SSE__ || __AVX__

                ialphap += 2;
                rows0p += 3;
                rows1p += 3;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + stride * (dy);

#if __ARM_NEON || __SSE__ || __AVX__
        int nn = (w * 3) >> 3;
#else
        int nn = 0;
#endif
        int remain = (w * 3) - (nn << 3);

#if __ARM_NEON || __SSE__ || __AVX__
#if __aarch64__ || __SSE__ || __AVX__
        int16x4_t _b0 = vdup_n_s16(b0);
        int16x4_t _b1 = vdup_n_s16(b1);
        int32x4_t _v2 = vdupq_n_s32(2);
        for (; nn > 0; nn--)
        {
            int16x4_t _rows0p_sr4 = vld1_s16(rows0p);
            int16x4_t _rows1p_sr4 = vld1_s16(rows1p);
            int16x4_t _rows0p_1_sr4 = vld1_s16(rows0p + 4);
            int16x4_t _rows1p_1_sr4 = vld1_s16(rows1p + 4);

            int32x4_t _rows0p_sr4_mb0 = vmull_s16(_rows0p_sr4, _b0);
            int32x4_t _rows1p_sr4_mb1 = vmull_s16(_rows1p_sr4, _b1);
            int32x4_t _rows0p_1_sr4_mb0 = vmull_s16(_rows0p_1_sr4, _b0);
            int32x4_t _rows1p_1_sr4_mb1 = vmull_s16(_rows1p_1_sr4, _b1);

            int32x4_t _acc = _v2;
            _acc = vsraq_n_s32(_acc, _rows0p_sr4_mb0, 16);
            _acc = vsraq_n_s32(_acc, _rows1p_sr4_mb1, 16);

            int32x4_t _acc_1 = _v2;
            _acc_1 = vsraq_n_s32(_acc_1, _rows0p_1_sr4_mb0, 16);
            _acc_1 = vsraq_n_s32(_acc_1, _rows1p_1_sr4_mb1, 16);

            int16x4_t _acc16 = vshrn_n_s32(_acc, 2);
            int16x4_t _acc16_1 = vshrn_n_s32(_acc_1, 2);

            uint8x8_t _D = vqmovun_s16(vcombine_s16(_acc16, _acc16_1));

            vst1_u8(Dp, _D);

            Dp += 8;
            rows0p += 8;
            rows1p += 8;
        }
#else
        if (nn > 0)
        {
            asm volatile(
                "vdup.s16   d16, %8         \n"
                "mov        r4, #2          \n"
                "vdup.s16   d17, %9         \n"
                "vdup.s32   q12, r4         \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "0:                         \n"
                "vmull.s16  q0, d2, d16     \n"
                "vmull.s16  q1, d3, d16     \n"
                "vorr.s32   q10, q12, q12   \n"
                "vorr.s32   q11, q12, q12   \n"
                "vmull.s16  q2, d6, d17     \n"
                "vmull.s16  q3, d7, d17     \n"
                "vsra.s32   q10, q0, #16    \n"
                "vsra.s32   q11, q1, #16    \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "vsra.s32   q10, q2, #16    \n"
                "vsra.s32   q11, q3, #16    \n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "vshrn.s32  d20, q10, #2    \n"
                "vshrn.s32  d21, q11, #2    \n"
                "vqmovun.s16 d20, q10        \n"
                "vst1.8     {d20}, [%2]!    \n"
                "subs       %3, #1          \n"
                "bne        0b              \n"
                "sub        %0, #16         \n"
                "sub        %1, #16         \n"
                : "=r"(rows0p), // %0
                "=r"(rows1p), // %1
                "=r"(Dp),     // %2
                "=r"(nn)      // %3
                : "0"(rows0p),
                "1"(rows1p),
                "2"(Dp),
                "3"(nn),
                "r"(b0), // %8
                "r"(b1)  // %9
                : "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12");
        }
#endif // __aarch64__ || __SSE__ || __AVX__
#endif // __ARM_NEON || __SSE__ || __AVX__
        for (; remain; --remain)
        {
            //             D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(((short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >> 2);
        }

        ibeta += 2;
    }

    delete[] buf;
}

static void resize_bilinear_c4(const unsigned char* src, int srcw, int srch, int srcstride, unsigned char* dst, int w, int h, int stride)
{
    const int INTER_RESIZE_COEF_BITS = 11;
    const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;
    //     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = new int[w + h + w + h];

    int* xofs = buf;     //new int[w];
    int* yofs = buf + w; //new int[h];

    short* ialpha = (short*)(buf + w + h);    //new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w); //new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)::std::min(::std::max((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = static_cast<int>(floor(fx));
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx * 4;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 = fx * INTER_RESIZE_COEF_SCALE;

        ialpha[dx * 2] = SATURATE_CAST_SHORT(a0);
        ialpha[dx * 2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = static_cast<int>(floor(fy));
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 = fy * INTER_RESIZE_COEF_SCALE;

        ibeta[dy * 2] = SATURATE_CAST_SHORT(b0);
        ibeta[dy * 2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    ImGui::ImMat rowsbuf0(w * 4, (size_t)2u);
    ImGui::ImMat rowsbuf1(w * 4, (size_t)2u);
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++)
    {
        sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 1)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S1p = S1 + sx;
#if __ARM_NEON || __SSE__ || __AVX__
                int16x4_t _a0 = vdup_n_s16(a0);
                int16x4_t _a1 = vdup_n_s16(a1);
                uint8x8_t _S1 = vld1_u8(S1p);
                int16x8_t _S116 = vreinterpretq_s16_u16(vmovl_u8(_S1));
                int16x4_t _S1low = vget_low_s16(_S116);
                int16x4_t _S1high = vget_high_s16(_S116);
                int32x4_t _rows1 = vmull_s16(_S1low, _a0);
                _rows1 = vmlal_s16(_rows1, _S1high, _a1);
                int16x4_t _rows1_sr4 = vshrn_n_s32(_rows1, 4);
                vst1_s16(rows1p, _rows1_sr4);
#else
                rows1p[0] = (S1p[0] * a0 + S1p[4] * a1) >> 4;
                rows1p[1] = (S1p[1] * a0 + S1p[5] * a1) >> 4;
                rows1p[2] = (S1p[2] * a0 + S1p[6] * a1) >> 4;
                rows1p[3] = (S1p[3] * a0 + S1p[7] * a1) >> 4;
#endif // __ARM_NEON || __SSE__ || __AVX__

                ialphap += 2;
                rows1p += 4;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char* S0 = src + srcstride * (sy);
            const unsigned char* S1 = src + srcstride * (sy + 1);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;
#if __ARM_NEON || __SSE__ || __AVX__
                int16x4_t _a0 = vdup_n_s16(a0);
                int16x4_t _a1 = vdup_n_s16(a1);
                uint8x8_t _S0 = vld1_u8(S0p);
                uint8x8_t _S1 = vld1_u8(S1p);
                int16x8_t _S016 = vreinterpretq_s16_u16(vmovl_u8(_S0));
                int16x8_t _S116 = vreinterpretq_s16_u16(vmovl_u8(_S1));
                int16x4_t _S0low = vget_low_s16(_S016);
                int16x4_t _S1low = vget_low_s16(_S116);
                int16x4_t _S0high = vget_high_s16(_S016);
                int16x4_t _S1high = vget_high_s16(_S116);
                int32x4_t _rows0 = vmull_s16(_S0low, _a0);
                int32x4_t _rows1 = vmull_s16(_S1low, _a0);
                _rows0 = vmlal_s16(_rows0, _S0high, _a1);
                _rows1 = vmlal_s16(_rows1, _S1high, _a1);
                int16x4_t _rows0_sr4 = vshrn_n_s32(_rows0, 4);
                int16x4_t _rows1_sr4 = vshrn_n_s32(_rows1, 4);
                vst1_s16(rows0p, _rows0_sr4);
                vst1_s16(rows1p, _rows1_sr4);
#else
                rows0p[0] = (S0p[0] * a0 + S0p[4] * a1) >> 4;
                rows0p[1] = (S0p[1] * a0 + S0p[5] * a1) >> 4;
                rows0p[2] = (S0p[2] * a0 + S0p[6] * a1) >> 4;
                rows0p[3] = (S0p[3] * a0 + S0p[7] * a1) >> 4;
                rows1p[0] = (S1p[0] * a0 + S1p[4] * a1) >> 4;
                rows1p[1] = (S1p[1] * a0 + S1p[5] * a1) >> 4;
                rows1p[2] = (S1p[2] * a0 + S1p[6] * a1) >> 4;
                rows1p[3] = (S1p[3] * a0 + S1p[7] * a1) >> 4;
#endif // __ARM_NEON || __SSE__ || __AVX__

                ialphap += 2;
                rows0p += 4;
                rows1p += 4;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + stride * (dy);

#if __ARM_NEON || __SSE__ || __AVX__
        int nn = (w * 4) >> 3;
#else
        int nn = 0;
#endif
        int remain = (w * 4) - (nn << 3);

#if __ARM_NEON || __SSE__ || __AVX__
#if __aarch64__ || __SSE__ || __AVX__
        int16x4_t _b0 = vdup_n_s16(b0);
        int16x4_t _b1 = vdup_n_s16(b1);
        int32x4_t _v2 = vdupq_n_s32(2);
        for (; nn > 0; nn--)
        {
            int16x4_t _rows0p_sr4 = vld1_s16(rows0p);
            int16x4_t _rows1p_sr4 = vld1_s16(rows1p);
            int16x4_t _rows0p_1_sr4 = vld1_s16(rows0p + 4);
            int16x4_t _rows1p_1_sr4 = vld1_s16(rows1p + 4);

            int32x4_t _rows0p_sr4_mb0 = vmull_s16(_rows0p_sr4, _b0);
            int32x4_t _rows1p_sr4_mb1 = vmull_s16(_rows1p_sr4, _b1);
            int32x4_t _rows0p_1_sr4_mb0 = vmull_s16(_rows0p_1_sr4, _b0);
            int32x4_t _rows1p_1_sr4_mb1 = vmull_s16(_rows1p_1_sr4, _b1);

            int32x4_t _acc = _v2;
            _acc = vsraq_n_s32(_acc, _rows0p_sr4_mb0, 16);
            _acc = vsraq_n_s32(_acc, _rows1p_sr4_mb1, 16);

            int32x4_t _acc_1 = _v2;
            _acc_1 = vsraq_n_s32(_acc_1, _rows0p_1_sr4_mb0, 16);
            _acc_1 = vsraq_n_s32(_acc_1, _rows1p_1_sr4_mb1, 16);

            int16x4_t _acc16 = vshrn_n_s32(_acc, 2);
            int16x4_t _acc16_1 = vshrn_n_s32(_acc_1, 2);

            uint8x8_t _D = vqmovun_s16(vcombine_s16(_acc16, _acc16_1));

            vst1_u8(Dp, _D);

            Dp += 8;
            rows0p += 8;
            rows1p += 8;
        }
#else
        if (nn > 0)
        {
            asm volatile(
                "vdup.s16   d16, %8         \n"
                "mov        r4, #2          \n"
                "vdup.s16   d17, %9         \n"
                "vdup.s32   q12, r4         \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "0:                         \n"
                "vmull.s16  q0, d2, d16     \n"
                "vmull.s16  q1, d3, d16     \n"
                "vorr.s32   q10, q12, q12   \n"
                "vorr.s32   q11, q12, q12   \n"
                "vmull.s16  q2, d6, d17     \n"
                "vmull.s16  q3, d7, d17     \n"
                "vsra.s32   q10, q0, #16    \n"
                "vsra.s32   q11, q1, #16    \n"
                "pld        [%0, #128]      \n"
                "vld1.s16   {d2-d3}, [%0 :128]!\n"
                "vsra.s32   q10, q2, #16    \n"
                "vsra.s32   q11, q3, #16    \n"
                "pld        [%1, #128]      \n"
                "vld1.s16   {d6-d7}, [%1 :128]!\n"
                "vshrn.s32  d20, q10, #2    \n"
                "vshrn.s32  d21, q11, #2    \n"
                "vqmovun.s16 d20, q10        \n"
                "vst1.8     {d20}, [%2]!    \n"
                "subs       %3, #1          \n"
                "bne        0b              \n"
                "sub        %0, #16         \n"
                "sub        %1, #16         \n"
                : "=r"(rows0p), // %0
                "=r"(rows1p), // %1
                "=r"(Dp),     // %2
                "=r"(nn)      // %3
                : "0"(rows0p),
                "1"(rows1p),
                "2"(Dp),
                "3"(nn),
                "r"(b0), // %8
                "r"(b1)  // %9
                : "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12");
        }
#endif // __aarch64__ || __SSE__ || __AVX__
#endif // __ARM_NEON || __SSE__ || __AVX__
        for (; remain; --remain)
        {
            //             D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(((short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2) >> 2);
        }

        ibeta += 2;
    }

    delete[] buf;
}

static void resize_bilinear_c1(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    return resize_bilinear_c1(src, srcw, srch, srcw, dst, w, h, w);
}

static void resize_bilinear_c2(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    return resize_bilinear_c2(src, srcw, srch, srcw * 2, dst, w, h, w * 2);
}

static void resize_bilinear_c3(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    return resize_bilinear_c3(src, srcw, srch, srcw * 3, dst, w, h, w * 3);
}

static void resize_bilinear_c4(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    return resize_bilinear_c4(src, srcw, srch, srcw * 4, dst, w, h, w * 4);
}

ImGui::ImMat ImGui::MatResize(const ImGui::ImMat& mat, const ImSize size, float sw, float sh)
{
    ImGui::ImMat dst;
    int srcw = mat.w;
    int srch = mat.h;

    int w = size.w;
    int h = size.h;

    if (w == 0 || h == 0)
    {
        w = srcw * sw;
        h = srch * sh;
    }

    if (w == 0 || h == 0)
        return dst;

    if (w == srcw && h == srch)
    {
        dst = mat.clone();
        return dst;
    }

    dst.create(w, h, mat.c, 1u, mat.c);

    if (mat.c == 1)
        resize_bilinear_c1((const unsigned char*)mat.data, srcw, srch, (unsigned char*)dst.data, w, h);
    else if (mat.c == 2)
        resize_bilinear_c2((const unsigned char*)mat.data, srcw, srch, (unsigned char*)dst.data, w, h);
    else if (mat.c == 3)
        resize_bilinear_c3((const unsigned char*)mat.data, srcw, srch, (unsigned char*)dst.data, w, h);
    else if (mat.c == 4)
        resize_bilinear_c4((const unsigned char*)mat.data, srcw, srch, (unsigned char*)dst.data, w, h);

    return dst;
}

ImGui::ImMat ImGui::GrayToImage(const ImGui::ImMat& mat)
{
    ImMat dst;
    if (mat.c != 1 || mat.device != IM_DD_CPU)
        return dst;
    dst.create_type(mat.w, mat.h, 4, mat.type);
    dst.elempack = 4;
    for (int row = 0; row < mat.h; row++)
    {
        for (int col = 0; col < mat.w; col++)
        {
            switch (mat.type)
            {
                case IM_DT_INT8:
                    dst.at<uint8_t>(col, row, 0) = mat.at<uint8_t>(col, row);
                    dst.at<uint8_t>(col, row, 1) = mat.at<uint8_t>(col, row);
                    dst.at<uint8_t>(col, row, 2) = mat.at<uint8_t>(col, row);
                    dst.at<uint8_t>(col, row, 3) = UINT8_MAX;
                break;
                case IM_DT_INT16:
                    dst.at<uint16_t>(col, row, 0) = mat.at<uint16_t>(col, row);
                    dst.at<uint16_t>(col, row, 1) = mat.at<uint16_t>(col, row);
                    dst.at<uint16_t>(col, row, 2) = mat.at<uint16_t>(col, row);
                    dst.at<uint16_t>(col, row, 3) = UINT16_MAX;
                break;
                case IM_DT_INT32:
                    dst.at<uint32_t>(col, row, 0) = mat.at<uint32_t>(col, row);
                    dst.at<uint32_t>(col, row, 1) = mat.at<uint32_t>(col, row);
                    dst.at<uint32_t>(col, row, 2) = mat.at<uint32_t>(col, row);
                    dst.at<uint32_t>(col, row, 3) = UINT32_MAX;
                break;
                case IM_DT_INT64:
                    dst.at<uint64_t>(col, row, 0) = mat.at<uint64_t>(col, row);
                    dst.at<uint64_t>(col, row, 1) = mat.at<uint64_t>(col, row);
                    dst.at<uint64_t>(col, row, 2) = mat.at<uint64_t>(col, row);
                    dst.at<uint64_t>(col, row, 3) = UINT64_MAX;
                break;
                case IM_DT_FLOAT16:
                    dst.at<uint16_t>(col, row, 0) = mat.at<uint16_t>(col, row);
                    dst.at<uint16_t>(col, row, 1) = mat.at<uint16_t>(col, row);
                    dst.at<uint16_t>(col, row, 2) = mat.at<uint16_t>(col, row);
                    dst.at<uint16_t>(col, row, 3) = im_float32_to_float16(1.0);
                break;
                case IM_DT_FLOAT32:
                    dst.at<float>(col, row, 0) = mat.at<float>(col, row);
                    dst.at<float>(col, row, 1) = mat.at<float>(col, row);
                    dst.at<float>(col, row, 2) = mat.at<float>(col, row);
                    dst.at<float>(col, row, 3) = 1.0f;
                break;
                case IM_DT_FLOAT64:
                    dst.at<double>(col, row, 0) = mat.at<double>(col, row);
                    dst.at<double>(col, row, 1) = mat.at<double>(col, row);
                    dst.at<double>(col, row, 2) = mat.at<double>(col, row);
                    dst.at<double>(col, row, 3) = 1.0;
                break;
                default: break;
            }
        }
    }
    return dst;
}


static const unsigned char* GetTextData(const ImWchar c, ImVec2& size, ImVec4& rect, int& output_stride, int& char_width, int& char_height)
{
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    float scale_x = c < 0x80 ? 2.0 : 1.0;
    float scale_y = c < 0x80 ? 2.0 : 1.0;
    unsigned char* bitmap;
    int _out_width, _out_height;
    atlas->GetTexDataAsAlpha8(&bitmap, &_out_width, &_out_height);
    const ImFontGlyph* glyph = ImGui::GetCurrentContext()->Font->FindGlyph(c);
    if (glyph == NULL)
        return nullptr;
    const int U1 = (int)(glyph->U1 * _out_width);
    const int U0 = (int)(glyph->U0 * _out_width);
    const int V1 = (int)(glyph->V1 * _out_height);
    const int V0 = (int)(glyph->V0 * _out_height);
    const unsigned char * ptr = &bitmap[_out_width * V0 + U0];
    output_stride = _out_width;
    size.x = U1 - U0;
    size.y = V1 - V0;
    rect.x = glyph->X0 * scale_x;
    rect.y = glyph->Y0 * scale_y;
    rect.z = glyph->X1 * scale_x;
    rect.w = glyph->Y1 * scale_y;
    char_width = rect.x + glyph->AdvanceX * scale_x;
    char_height = glyph->Y0 * scale_y + V1 - V0;
    return ptr;
}

static const ImVec2 GetTextSize(const ImWchar c)
{
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    const ImFontGlyph* glyph = ImGui::GetCurrentContext()->Font->FindGlyph(c);
    if (glyph == NULL)
        return ImVec2(0, 0);
    const int V1 = (int)(glyph->V1 * atlas->TexHeight);
    const int V0 = (int)(glyph->V0 * atlas->TexHeight);
    float scale_x = c < 0x80 ? 2.0 : 1.0;
    float scale_y = c < 0x80 ? 2.0 : 1.0;
    float width = glyph->X0 * scale_x + glyph->AdvanceX * scale_x;
    float height = glyph->Y0 * scale_y + V1 - V0;
    return ImVec2(width, height);
}

void ImGui::DrawTextToMat(ImGui::ImMat& mat, const ImPoint pos, const char* str, const ImPixel& color, float scale)
{
    int start_x = pos.x;
    int start_y = pos.y;
    const char* str_ptr = str;
    const char* str_end = str_ptr + strlen(str);
    while (str_ptr < str_end)
    {
        unsigned int c = *str_ptr;
        if (c < 0x80)
            str_ptr += 1;
        else
            str_ptr += ImTextCharFromUtf8(&c, str_ptr, str_end);
        if (c < 32)
        {
            if (c == '\n')
            {
                start_x = pos.x;
                start_y += ImGui::GetFontSize() * scale;
                continue;
            }
            if (c == '\r')
                continue;
        }
        float scale_internal = c < 0x80 ? 0.5 : 1.0;
        int output_stride = 0, char_width = 0, char_height = 0;
        ImVec2 size = {0, 0};
        ImVec4 rect = {0, 0, 0, 0};
        const unsigned char* out_data = GetTextData(c, size, rect, output_stride, char_width, char_height);
        if (out_data && output_stride)
        {
            ImGui::ImMat char_mat(char_width, char_height, 4, 1u, 4);
            float x1 = rect.x;
            float x2 = rect.z;
            float y1 = rect.y;
            float y2 = rect.w;
            
            for (int x = 0; x < size.x; x++)
            {
                for (int y = 0; y < size.y; y++)
                {
                    const unsigned char alpha = out_data[y * output_stride + x];
                    char_mat.draw_dot(ImPoint(x + x1, y + y1), ImPixel(color.r, color.g, color.b, alpha / 255.0));
                }
            }
            auto scale_mat = ImGui::MatResize(char_mat, ImSize(char_width * scale * scale_internal, ImGui::GetFontSize() * scale));
            ImGui::ImageMatCopyTo(scale_mat, mat, ImPoint(start_x, start_y));
            start_x += char_width * scale * scale_internal;
        }
    }
}

ImGui::ImMat ImGui::CreateTextMat(const char* str, const ImPixel& color, float scale)
{
    ImGui::ImMat dst;
    if (!str || strlen(str) == 0)
        return dst;

    int lines = 1;
    float line_width = 0;
    float max_line_width = -1;
    const char* str_ptr = str;
    const char* str_end = str_ptr + strlen(str);
    while (str_ptr < str_end)
    {
        unsigned int c = *str_ptr;
        if (c < 0x80)
            str_ptr += 1;
        else
            str_ptr += ImTextCharFromUtf8(&c, str_ptr, str_end);
        if (c < 32)
        {
            if (c == '\n')
            {
                if (max_line_width < line_width) max_line_width = line_width;
                line_width = 0;
                lines++;
                continue;
            }
            if (c == '\r')
                continue;
        }
        float scale_internal = c < 0x80 ? 0.5 : 1.0;
        auto char_size = GetTextSize(c);
        line_width += char_size.x * scale_internal;
        if (max_line_width < line_width) max_line_width = line_width;
    }

    float text_height = ImGui::GetFontSize() * lines * scale;
    float text_width = max_line_width * scale;
    dst.create_type(ceil(text_width), ceil(text_height), 4, IM_DT_INT8);
    dst.elempack = 4;
    ImGui::DrawTextToMat(dst, ImPoint(0, 0), str, color, scale);
    return dst;
}

void ImGui::ImageMatCopyTo(const ImGui::ImMat& src, ImGui::ImMat& dst, ImPoint pos)
{
    if (src.empty() || dst.empty())
        return;
    ImPixel pixel;
    for (int x = 0; x < src.w; x++)
    {
        for (int y = 0; y < src.h; y++)
        {
            src.get_pixel(x, y, pixel);
            if (pixel.a > 0)
                dst.draw_dot((int)(pos.x + x), (int)(pos.y + y), pixel);
        }
    }
}

// platform folders
// https://github.com/sago007/PlatformFolders
#ifdef _WIN32
class FreeCoTaskMemory {
	LPWSTR pointer = NULL;
public:
	explicit FreeCoTaskMemory(LPWSTR pointer) : pointer(pointer) {};
	~FreeCoTaskMemory() {
		CoTaskMemFree(pointer);
	}
};
static std::string win32_utf16_to_utf8(const wchar_t* wstr) {
	std::string res;
	// If the 6th parameter is 0 then WideCharToMultiByte returns the number of bytes needed to store the result.
	int actualSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	if (actualSize > 0) {
		//If the converted UTF-8 string could not be in the initial buffer. Allocate one that can hold it.
		std::vector<char> buffer(actualSize);
		actualSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], static_cast<int>(buffer.size()), nullptr, nullptr);
		res = buffer.data();
	}
	if (actualSize == 0) {
		// WideCharToMultiByte return 0 for errors.
		throw std::runtime_error("UTF16 to UTF8 failed with error code: " + std::to_string(GetLastError()));
	}
	return res;
}
static std::string GetKnownWindowsFolder(REFKNOWNFOLDERID folderId, const char* errorMsg) {
	LPWSTR wszPath = NULL;
	HRESULT hr;
	hr = SHGetKnownFolderPath(folderId, KF_FLAG_CREATE, NULL, &wszPath);
	FreeCoTaskMemory scopeBoundMemory(wszPath);

	if (!SUCCEEDED(hr)) {
		throw std::runtime_error(errorMsg);
	}
	return win32_utf16_to_utf8(wszPath);
}
static std::string GetAppData() {
	return GetKnownWindowsFolder(FOLDERID_RoamingAppData, "RoamingAppData could not be found");
}
static std::string GetAppDataCommon() {
	return GetKnownWindowsFolder(FOLDERID_ProgramData, "ProgramData could not be found");
}

static std::string GetAppDataLocal() {
	return GetKnownWindowsFolder(FOLDERID_LocalAppData, "LocalAppData could not be found");
}
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
#include <fstream>
static void PlatformFoldersAddFromFile(const std::string& filename, std::map<std::string, std::string>& folders) {
	std::ifstream infile(filename.c_str());
	std::string line;
	while (std::getline(infile, line)) {
		if (line.length() == 0 || line.at(0) == '#' || line.substr(0, 4) != "XDG_" || line.find("_DIR") == std::string::npos) {
			continue;
		}
		try {
			std::size_t splitPos = line.find('=');
			std::string key = line.substr(0, splitPos);
			std::size_t valueStart = line.find('"', splitPos);
			std::size_t valueEnd = line.find('"', valueStart+1);
			std::string value = line.substr(valueStart+1, valueEnd - valueStart - 1);
			folders[key] = value;
		}
		catch (std::exception&  e) {
			std::cerr << "WARNING: Failed to process \"" << line << "\" from \"" << filename << "\". Error: "<< e.what() << "\n";
			continue;
		}
	}
}

static void PlatformFoldersFillData(std::map<std::string, std::string>& folders) {
	folders["XDG_DOCUMENTS_DIR"] = "$HOME/Documents";
	folders["XDG_DESKTOP_DIR"] = "$HOME/Desktop";
	folders["XDG_DOWNLOAD_DIR"] = "$HOME/Downloads";
	folders["XDG_MUSIC_DIR"] = "$HOME/Music";
	folders["XDG_PICTURES_DIR"] = "$HOME/Pictures";
	folders["XDG_PUBLICSHARE_DIR"] = "$HOME/Public";
	folders["XDG_TEMPLATES_DIR"] = "$HOME/.Templates";
	folders["XDG_VIDEOS_DIR"] = "$HOME/Videos";
	PlatformFoldersAddFromFile( ImGuiHelper::getConfigHome()+"/user-dirs.dirs", folders);
	for (std::map<std::string, std::string>::iterator itr = folders.begin() ; itr != folders.end() ; ++itr ) {
		std::string& value = itr->second;
		if (value.compare(0, 5, "$HOME") == 0) {
			value = ImGuiHelper::home_path() + value.substr(6, std::string::npos);
		}
	}
}

static void throwOnRelative(const char* envName, const char* envValue) {
	if (envValue[0] != '/') {
		char buffer[200];
		std::snprintf(buffer, sizeof(buffer), "Environment \"%s\" does not start with an '/'. XDG specifies that the value must be absolute. The current value is: \"%s\"", envName, envValue);
		throw std::runtime_error(buffer);
	}
}

static std::string getLinuxFolderDefault(const char* envName, const char* defaultRelativePath) {
	std::string res;
	const char* tempRes = std::getenv(envName);
	if (tempRes) {
		throwOnRelative(envName, tempRes);
		res = tempRes;
		return res;
	}
	res = ImGuiHelper::home_path() + defaultRelativePath;
	return res;
}

#endif

namespace ImGuiHelper
{
std::string getDataHome() {
#ifdef _WIN32
	return GetAppData();
#elif defined(__APPLE__)
	return home_path()+"Library/Application Support";
#else
	return getLinuxFolderDefault("XDG_DATA_HOME", ".local/share");
#endif
}

std::string getConfigHome() {
#ifdef _WIN32
	return GetAppData();
#elif defined(__APPLE__)
	return home_path()+"Library/Application Support";
#else
	return getLinuxFolderDefault("XDG_CONFIG_HOME", ".config");
#endif
}

std::string getCacheDir() {
#ifdef _WIN32
	return GetAppDataLocal();
#elif defined(__APPLE__)
	return home_path()+"Library/Caches";
#else
	return getLinuxFolderDefault("XDG_CACHE_HOME", ".cache");
#endif
}

std::string getStateDir() {
#ifdef _WIN32
	return GetAppDataLocal();
#elif defined(__APPLE__)
	return home_path()+"Library/Application Support";
#else
	return getLinuxFolderDefault("XDG_STATE_HOME", ".local/state");
#endif
}

std::string getDocumentsFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Documents, "Failed to find My Documents folder");
#elif defined(__APPLE__)
	return home_path()+"Documents";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_DOCUMENTS_DIR"];
#endif
}

std::string getDesktopFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Desktop, "Failed to find Desktop folder");
#elif defined(__APPLE__)
	return home_path()+"Desktop";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_DESKTOP_DIR"];
#endif
}

std::string getPicturesFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Pictures, "Failed to find My Pictures folder");
#elif defined(__APPLE__)
	return home_path()+"Pictures";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_PICTURES_DIR"];
#endif
}

std::string getPublicFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Public, "Failed to find the Public folder");
#elif defined(__APPLE__)
	return home_path()+"Public";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_PUBLICSHARE_DIR"];
#endif
}

std::string getDownloadFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Downloads, "Failed to find My Downloads folder");
#elif defined(__APPLE__)
	return home_path()+"Downloads";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_DOWNLOAD_DIR"];
#endif
}

std::string getMusicFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Music, "Failed to find My Music folder");
#elif defined(__APPLE__)
	return home_path()+"Music";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_MUSIC_DIR"];
#endif
}

std::string getVideoFolder() {
#ifdef _WIN32
	return GetKnownWindowsFolder(FOLDERID_Videos, "Failed to find My Video folder");
#elif defined(__APPLE__)
	return home_path()+"Movies";
#else
    std::map<std::string, std::string> folders;
    PlatformFoldersFillData(folders);
	return folders["XDG_VIDEOS_DIR"];
#endif
}
} // namespace ImGuiHelper