#pragma once

#include <vkRender.h>

#include "Texture.h"

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

    static vk::ImageView newImageView(const vk::Image &image, vk::Format format, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor, uint32_t mipLevels = 1);
    vk::RenderPassBeginInfo newRenderPassBeginInfo(int currentFrame) const;
    std::vector<vk::Framebuffer> &getFrameBuffers() {return framebuffers_;}
    vk::RenderPass &getRenderPass() {return renderPass_;}
    vk::SwapchainKHR &get() {return swapchain_;}

    void release() const;
    void releaseSwapchain() const;

    SwapchainInfo info{};
private:
    std::vector<vk::ImageView> imageViews;
    std::vector<vk::Framebuffer> framebuffers_;
    vk::SwapchainKHR swapchain_;
    vk::RenderPass renderPass_;

    Texture *depthTexture = nullptr;
    
    void queryInfo();
    void createSwapChain();
    void createFramebuffer();
    void createRenderPass();
};

}
