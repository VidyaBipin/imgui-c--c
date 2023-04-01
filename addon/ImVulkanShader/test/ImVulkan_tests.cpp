#include "ImVulkanShader.h"
#include <iostream>
#include <unistd.h>
#include <ALM_vulkan.h>
#include <Bilateral_vulkan.h>
#include <Box.h>
#include <Brightness_vulkan.h>

#include <AlphaBlending_vulkan.h>
#include <BookFlip_vulkan.h>
#include <Bounce_vulkan.h>
#include <BowTie_vulkan.h>

#include <CIE_vulkan.h>
#include <Harris_vulkan.h>
#include <Histogram_vulkan.h>
#include <Vector_vulkan.h>
#include <Waveform_vulkan.h>

#define TEST_WIDTH  1920
#define TEST_HEIGHT 1080
#define TEST_GPU    0
#define TEST_TIMES  10
static ImGui::ImMat mat1, mat2, dstmat;

static void test_filters(int gpu, int times)
{
    if (mat1.empty())
        return;
    std::cout << "Filters:" << std::endl;
    // test ALM
    ImGui::ALM_vulkan* alm = new ImGui::ALM_vulkan(gpu);
    if (alm)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = alm->filter(mat1, dstmat);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (ALM)" << std::endl;
        delete alm;
    }

    // test Bilateral
    ImGui::Bilateral_vulkan* bilateral = new ImGui::Bilateral_vulkan(gpu);
    if (bilateral)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = bilateral->filter(mat1, dstmat, 5, 10.f, 10.f);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Bilateral)" << std::endl;
        delete bilateral;
    }

    // test Box
    ImGui::BoxBlur_vulkan* box = new ImGui::BoxBlur_vulkan(gpu);
    if (box)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = box->filter(mat1, dstmat);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (BoxBlur)" << std::endl;
        delete box;
    }

    // test Brightness
    ImGui::Brightness_vulkan* bright = new ImGui::Brightness_vulkan(gpu);
    if (bright)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = bright->filter(mat1, dstmat, 0.5);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Brightness)" << std::endl;
        delete bright;
    }

    // TODO::Dicky add more filter test
}

static void test_fusions(int gpu, int times)
{
    if (mat1.empty() || mat2.empty())
        return;

    std::cout << "Fusions:" << std::endl;
    ImPixel color = ImPixel(0.0f, 0.0f, 0.0f, 0.6f);
    // test AlphaBlending
    ImGui::AlphaBlending_vulkan *alphablending = new ImGui::AlphaBlending_vulkan(gpu);
    if (alphablending)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = alphablending->blend(mat1, mat2, dstmat, 0.5f);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (AlphaBlending)" << std::endl;
        delete alphablending;
    }

    // test BookFlip
    ImGui::BookFlip_vulkan * boolflip = new ImGui::BookFlip_vulkan(gpu);
    if (boolflip)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = boolflip->transition(mat1, mat2, dstmat, 0.5f);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (BookFlip)" << std::endl;
        delete boolflip;
    }

    // test ounce
    ImGui::Bounce_vulkan * bounce = new ImGui::Bounce_vulkan(gpu);
    if (bounce)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = bounce->transition(mat1, mat2, dstmat, 0.5f, color, 0.075f, 3.f);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Bounce)" << std::endl;
        delete bounce;
    }

    // test BowTie
    ImGui::BowTieHorizontal_vulkan * bowtie = new ImGui::BowTieHorizontal_vulkan(gpu);
    if (bowtie)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = bowtie->transition(mat1, mat2, dstmat, 0.5f, 0);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (BowTie)" << std::endl;
        delete bowtie;
    }

    // TODO::Dicky add more fusion test
}

static void test_scopes(int gpu, int times)
{
    if (mat1.empty())
        return;
    std::cout << "Scopes:" << std::endl;
    // test CIE
    ImGui::CIE_vulkan* cie = new ImGui::CIE_vulkan(gpu);
    if (cie)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = cie->scope(mat1, dstmat);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (CIE)" << std::endl;
        delete cie;
    }
    
    // test Harris
    ImGui::Harris_vulkan* harris = new ImGui::Harris_vulkan(gpu);
    if (harris)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = harris->scope(mat1, dstmat, 3, 2.f, 0.1f, 0.04f, 5.f);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Harris)" << std::endl;
        delete harris;
    }

    // test Histogram
    ImGui::Histogram_vulkan* histogram = new ImGui::Histogram_vulkan(gpu);
    if (histogram)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = histogram->scope(mat1, dstmat);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Histogram)" << std::endl;
        delete histogram;
    }

    // test Vector
    ImGui::Vector_vulkan* vector = new ImGui::Vector_vulkan(gpu);
    if (vector)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = vector->scope(mat1, dstmat);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Vector)" << std::endl;
        delete vector;
    }

    // test Waveform
    ImGui::Waveform_vulkan* wave = new ImGui::Waveform_vulkan(gpu);
    if (wave)
    {
        double avg_time = 0, total_time = 0, max_time = -DBL_MAX, min_time = DBL_MAX; 
        for (int i = 0; i < times; i++)
        {
            auto shader_time = wave->scope(mat1, dstmat);
            if (shader_time > max_time) max_time = shader_time;
            if (shader_time < min_time) min_time = shader_time;
            total_time += shader_time;
        }
        avg_time = total_time / times;
        std::cout << "min:" << std::to_string(min_time) << "\tmax:" << std::to_string(max_time) << "\tavg:" << std::to_string(avg_time) << " (Waveform)" << std::endl;
        delete wave;
    }
}

static void test_others(int gpu, int times)
{
    // TODO::Dicky add others test
}

int main(int argc, char ** argv)
{
    int test_flags = argc > 1 ? 0 : 0xFFFFFFFF;
    int test_gpu = TEST_GPU;
    int test_loop = TEST_TIMES;
    int o = -1;
    const char *option_str = "aftsog:n:";
    while ((o = getopt(argc, argv, option_str)) != -1)
    {
        switch (o)
        {
            case 'a': test_flags = 0xFFFFFFFF; break;
            case 'f': test_flags |= 0x00000001; break;
            case 't': test_flags |= 0x00000002; break;
            case 's': test_flags |= 0x00000004; break;
            case 'o': test_flags |= 0x00000008; break;
            case 'g': test_gpu = atoi(optarg); break;
            case 'n': test_loop = atoi(optarg); break;
            default: break;
        }
    }

    ImGui::ImVulkanShaderInit();
    ImGui::VulkanDevice* vkdev = ImGui::get_gpu_device(test_gpu);
    std::string device_name = vkdev->info.device_name();

    mat1.create_type(TEST_WIDTH, TEST_HEIGHT, 4, IM_DT_INT8);
    mat1.randn(128.f, 128.f);
    
    // test ImMat to VkMat
    std::cout << "Test GPU:" << test_gpu << "\t(" << device_name << ")" << std::endl;
    std::cout << "Test Loop:" << test_loop << std::endl;
    std::cout << "Global:" << std::endl;
    ImGui::VkMat gmat;
    gmat.device_number = TEST_GPU;
    ImGui::ImVulkanImMatToVkMat(mat1, gmat);
    if (gmat.empty())
    {
        std::cout << "ImMat to VkMat failed!!!" << std::endl;
    }
    else
    {
        std::cout << "ImMat to VkMat passed" << std::endl;
        // test VkMat to ImMat
        ImGui::ImVulkanVkMatToImMat(gmat, mat2);
        if (mat2.empty())
            std::cout << "VkMat to ImMat failed!!!" << std::endl;
        else
            std::cout << "VkMat to ImMat passed" << std::endl;
        gmat.release();
    }

    if (mat2.empty())
        mat2.create_type(TEST_WIDTH, TEST_HEIGHT, 4, IM_DT_INT8);
    
    if (test_flags & 0x00000001) test_filters(test_gpu, test_loop);
    if (test_flags & 0x00000002) test_fusions(test_gpu, test_loop);
    if (test_flags & 0x00000004) test_scopes(test_gpu, test_loop);
    if (test_flags & 0x00000008) test_others(test_gpu, test_loop);

    ImGui::ImVulkanShaderClear();
    return 0;
}