#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <immat.h>
namespace ImGui {

IMGUI_API void ImGenerateOrUpdateTexture(ImTextureID& imtexid, int width, int height, int channels, const unsigned char* pixels, bool useMipmapsIfPossible, bool wraps, bool wrapt, bool minFilterNearest = false, bool magFilterNearest=false, bool is_immat=false);
IMGUI_API void ImGenerateOrUpdateTexture(ImTextureID& imtexid,int width, int height, int channels, const unsigned char* pixels, bool is_immat = false);
IMGUI_API ImTextureID ImCreateTexture(const void* data, int width, int height, double time_stamp = NAN, int bit_depth = 8);
IMGUI_API ImTextureID ImLoadTexture(const char* path);
IMGUI_API void ImLoadImageToMat(const char* path, ImMat& mat, bool gray = false);
IMGUI_API void ImDestroyTexture(ImTextureID texture);
IMGUI_API int ImGetTextureWidth(ImTextureID texture);
IMGUI_API int ImGetTextureHeight(ImTextureID texture);
IMGUI_API int ImGetTextureData(ImTextureID texture, void* data);
IMGUI_API ImPixel ImGetTexturePixel(ImTextureID texture, float x, float y);
IMGUI_API double ImGetTextureTimeStamp(ImTextureID texture);
IMGUI_API bool ImTextureToFile(ImTextureID texture, std::string path);
IMGUI_API void ImMatToTexture(ImMat mat, ImTextureID& texture);
IMGUI_API void ImTextureToMat(ImTextureID texture, ImMat& mat, ImVec2 offset = {}, ImVec2 size = {});
IMGUI_API void ImCopyToTexture(ImTextureID& imtexid, unsigned char* pixels, int width, int height, int channels, int offset_x, int offset_y, bool is_immat=false);
#if IMGUI_RENDERING_VULKAN && IMGUI_VULKAN_SHADER
IMGUI_API ImTextureID ImCreateTexture(VkImageMat & image, double time_stamp = NAN);
#endif
IMGUI_API void ImUpdateTextures(); // update internal textures, check need destroy texture and destroy it if we can
IMGUI_API void ImDestroyTextures(); // clean internal textures
IMGUI_API size_t ImGetTextureCount();

IMGUI_API void ImShowVideoWindow(ImDrawList *draw_list, ImTextureID texture, ImVec2 pos, ImVec2 size, float* offset_x = nullptr, float* offset_y = nullptr, float* tf_x = nullptr, float* tf_y = nullptr, bool bLandscape = true, bool out_border = false, const ImVec2& uvMin = ImVec2(0, 0), const ImVec2& uvMax = ImVec2(1, 1));

} // namespace ImGui