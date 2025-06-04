#include "Swapchain.h"

#include "Context.h"
#include "Device.h"

namespace vkRender
{

Swapchain* Swapchain::create()
{
    Swapchain *swapchain = new (std::nothrow) Swapchain();
    if (swapchain && swapchain->init())
    {
        return swapchain;
    }

    return nullptr;
}

bool Swapchain::init()
{
    queryInfo();
    createSwapChain();
    createFramebuffer();
    
    return true;
}

void Swapchain::reCreate()
{
    release();
    
    queryInfo();
    createSwapChain();
    createFramebuffer();
}

void Swapchain::release() const
{
    auto device = Device::getInstance()->getDevice();
    for (int i = 0; i < imageViews.size(); ++i)
    {
        auto imageView = imageViews[i];
        auto framebuffer = framebuffers_[i];
        
        device.destroyFramebuffer(framebuffer);
        device.destroyImageView(imageView);
    }
    device.destroySwapchainKHR(swapchain_);
}

void Swapchain::queryInfo()
{
    const auto& pDevice = Device::getInstance()->getPDevice();
    const auto& surface = Context::getInstance()->getSurface();
    auto swapChainSupport = pDevice.getSurfaceCapabilitiesKHR(surface);
    auto capabilities = pDevice.getSurfaceCapabilitiesKHR(surface);

    info.imageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount);
    
    auto Formats = pDevice.getSurfaceFormatsKHR(surface);
    info.format = Formats[0];
    for (auto format : Formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            info.format = format;
            break;
        }
    }

    vk::Extent2D actualExtent {640, 480};
    
    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    info.extent = actualExtent;
    info.transform = swapChainSupport.currentTransform;

    auto presentModes = pDevice.getSurfacePresentModesKHR(surface);
    info.presentMode = vk::PresentModeKHR::eFifo;
    
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            info.presentMode = presentMode;
            break;
        }
    }
}

void Swapchain::createSwapChain()
{
    const auto context = Device::getInstance();
    
    vk::SwapchainCreateInfoKHR createInfo;
    std::array indices = {context->indices_.graphicsFamily.value(), context->indices_.presentFamily.value()};
    
    createInfo
        .setSurface(Context::getInstance()->getSurface())
        .setImageArrayLayers(1)
        .setMinImageCount(info.imageCount)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setImageFormat(info.format.format)
        .setImageColorSpace(info.format.colorSpace)
        .setImageExtent(info.extent)
        .setPresentMode(info.presentMode)
        .setPreTransform(info.transform);

    if (context->indices_.equal())
    {
        createInfo.setQueueFamilyIndices(context->indices_.graphicsFamily.value());
    }
    else
    {
        createInfo.setQueueFamilyIndices(indices);
    }

    createInfo.setImageSharingMode(context->indices_.equal() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent);

    swapchain_ = context->getDevice().createSwapchainKHR(createInfo);
}

void Swapchain::createFramebuffer()
{
    auto device = Device::getInstance();

    std::vector images = device->getDevice().getSwapchainImagesKHR(swapchain_);

    imageViews.resize(images.size());
    framebuffers_.resize(imageViews.size());
    
    // Create imageViews 
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    
    for (int i = 0; i < images.size(); ++i)
    {
        vk::ImageViewCreateInfo imageviewInfo;
        imageviewInfo
            .setImage(images[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(vk::Format::eB8G8R8A8Srgb)
            .setComponents({})
            .setSubresourceRange(subresourceRange);

        imageViews[i] = device->getDevice().createImageView(imageviewInfo);

        vk::FramebufferCreateInfo framebufferInfo;
        std::array attachment = {imageViews[i]};
        
        framebufferInfo
            .setRenderPass(Context::getInstance()->getRenderPass())
            .setAttachmentCount(1)
            .setAttachments(attachment)
            .setWidth(info.extent.width)
            .setHeight(info.extent.height)
            .setLayers(1);
        
        framebuffers_[i] = device->getDevice().createFramebuffer(framebufferInfo);
    }
}

}



