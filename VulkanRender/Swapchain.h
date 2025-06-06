#pragma once

#include <vkRender.h>

namespace vkRender
{
struct SwapchainInfo
{
    uint32_t imageCount = 0;
    vk::SurfaceFormatKHR format;
    vk::Extent2D extent;
    vk::PresentModeKHR presentMode;
    vk::SurfaceTransformFlagBitsKHR transform;
};

class Swapchain
{
public:
    static Swapchain *create();

    bool init();
    void reCreate();

    static vk::ImageView newImageView(const vk::Image &image, vk::Format format);
    std::vector<vk::Framebuffer> &getFrameBuffers() {return framebuffers_;}
    vk::SwapchainKHR &get() {return swapchain_;}

    void release() const;

private:
    std::vector<vk::ImageView> imageViews;
    std::vector<vk::Framebuffer> framebuffers_;
    vk::SwapchainKHR swapchain_;
    SwapchainInfo info;
    
    void queryInfo();
    void createSwapChain();
    void createFramebuffer();
};

}
