#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "immat.h"

namespace ImGui 
{
class VKSHADER_API warpAffine_vulkan
{
public:
    warpAffine_vulkan(int gpu = -1);
    ~warpAffine_vulkan();

    virtual void filter(const ImMat& src, ImMat& dst, const ImMat& M, ImInterpolateMode type = IM_INTERPOLATE_NEAREST) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, const VkMat& M, ImInterpolateMode type) const;
};
} // namespace ImGui 