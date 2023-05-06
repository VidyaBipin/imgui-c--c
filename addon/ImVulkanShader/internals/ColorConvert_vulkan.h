#pragma once
#include <string>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"

namespace ImGui 
{
class VKSHADER_API ColorConvert_vulkan
{
public:
    ColorConvert_vulkan(int gpu = -1);
    ~ColorConvert_vulkan();

    double ConvertColorFormat(const ImMat& srcMat, ImMat& dstMat, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC);
    std::string GetError() const { return mErrMsg; }

    double RGBA2YUV(const ImMat& im_RGB, ImMat & im_YUV) const;
    double GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    double Conv(const ImMat& im, ImMat & om) const;

    double YUV2RGBA(const ImMat& im_YUV, ImMat & im_RGB, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC) const;
    double YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, ImMat & im_RGB, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC) const;

public:
    const VulkanDevice* vkdev;
    Pipeline * pipeline_yuv_rgb = nullptr;
    Pipeline * pipeline_rgb_yuv = nullptr;
    Pipeline * pipeline_gray_rgb = nullptr;
    Pipeline * pipeline_conv = nullptr;
    Pipeline * pipeline_y_u_v_rgb = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;

private:
    void upload_param(const VkMat& Im_Y, const VkMat& Im_U, const VkMat& Im_V, VkMat& dst, ImInterpolateMode type) const;
    void upload_param(const VkMat& Im_YUV, VkMat& dst, ImInterpolateMode type, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth) const;
    void upload_param(const VkMat& Im_RGB, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;
    void upload_param(const VkMat& Im, VkMat& dst, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    void upload_param(const VkMat& Im, VkMat& dst) const;

    bool UploadParam(const VkMat& src, VkMat& dst, ImInterpolateMode type);

    std::string mErrMsg;
};
} // namespace ImGui 