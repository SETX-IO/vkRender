#include "Swapchain.h"

#include "Context.h"
#include "Device.h"
#include "Memory/Memory.h"

namespace vkRender
{
US_VKN;

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

    depthTexture = Texture::createDepth(info.extent.width, info.extent.height);
    createRenderPass();
    createFramebuffer();
    
    return true;
}

void Swapchain::reCreate()
{
    Device::getInstance()->getDevice().waitIdle();
    
    releaseSwapchain();
    
    createSwapChain();
    createFramebuffer();
}

ImageView Swapchain::newImageView(const Image& image, Format format, ImageAspectFlags aspect)
{
    ImageSubresourceRange subresourceRange(aspect, 0, 1, 0, 1);
    
    ImageViewCreateInfo createInfo;
    createInfo
        .setImage(image)
        .setViewType(ImageViewType::e2D)
        .setFormat(format)
        .setSubresourceRange(subresourceRange);
    
    return Device::getInstance()->getDevice().createImageView(createInfo);
}

RenderPassBeginInfo Swapchain::newRenderPassBeginInfo(int currentFrame) const
{
    RenderPassBeginInfo beginInfo;
    beginInfo
        .setRenderPass(renderPass_)
        .setFramebuffer(framebuffers_[currentFrame]);
    
    return beginInfo;
}

void Swapchain::release() const
{
    auto device = Device::getInstance()->getDevice();
    
    releaseSwapchain();
    
    device.destroyRenderPass(renderPass_);
    depthTexture->release();
}

void Swapchain::releaseSwapchain() const
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
    const auto& pDevice = Device::getInstance()->getGPU();
    const auto& surface = Context::getInstance()->getSurface();
    auto swapChainSupport = pDevice.getSurfaceCapabilitiesKHR(surface);
    auto capabilities = pDevice.getSurfaceCapabilitiesKHR(surface);

    info.imageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount);
    
    auto Formats = pDevice.getSurfaceFormatsKHR(surface);
    info.format = Formats[0];
    for (auto format : Formats)
    {
        if (format.format == Format::eB8G8R8A8Srgb && format.colorSpace == ColorSpaceKHR::eSrgbNonlinear)
        {
            info.format = format;
            break;
        }
    }

    Extent2D actualExtent {640, 480};
    
    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    info.extent = actualExtent;
    info.transform = swapChainSupport.currentTransform;

    auto presentModes = pDevice.getSurfacePresentModesKHR(surface);
    info.presentMode = PresentModeKHR::eFifo;
    
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == PresentModeKHR::eMailbox)
        {
            info.presentMode = presentMode;
            break;
        }
    }
}

void Swapchain::createSwapChain()
{
    const auto device = Device::getInstance();
    
    SwapchainCreateInfoKHR createInfo;
    std::array indices = {device->indices_.graphicsFamily.value(), device->indices_.presentFamily.value()};
    
    createInfo
        .setSurface(Context::getInstance()->getSurface())
        .setImageArrayLayers(1)
        .setMinImageCount(info.imageCount)
        .setImageUsage(ImageUsageFlagBits::eColorAttachment)
        .setCompositeAlpha(CompositeAlphaFlagBitsKHR::eOpaque)
        .setImageFormat(info.format.format)
        .setImageColorSpace(info.format.colorSpace)
        .setImageExtent(info.extent)
        .setPresentMode(info.presentMode)
        .setPreTransform(info.transform);

    if (device->indices_.equal())
    {
        createInfo.setQueueFamilyIndices(device->indices_.graphicsFamily.value());
    }
    else
    {
        createInfo.setQueueFamilyIndices(indices);
    }

    createInfo.setImageSharingMode(device->indices_.equal() ? SharingMode::eExclusive : SharingMode::eConcurrent);

    swapchain_ = device->getDevice().createSwapchainKHR(createInfo);
}

void Swapchain::createFramebuffer()
{
    auto device = Device::getInstance();

    std::vector images = device->getDevice().getSwapchainImagesKHR(swapchain_);

    imageViews.resize(images.size());
    framebuffers_.resize(imageViews.size());
    
    for (int i = 0; i < images.size(); ++i)
    {
        imageViews[i] = newImageView(images[i], info.format.format);

        FramebufferCreateInfo framebufferInfo;
        std::array attachment = {imageViews[i], depthTexture->getView()};
        
        framebufferInfo
            .setRenderPass(renderPass_)
            .setAttachments(attachment)
            .setWidth(info.extent.width)
            .setHeight(info.extent.height)
            .setLayers(1);
        
        framebuffers_[i] = device->getDevice().createFramebuffer(framebufferInfo);
    }
}

void Swapchain::createRenderPass()
{
    RenderPassCreateInfo createInfo;
    
    AttachmentDescription colorAttachment;
    colorAttachment
        .setFormat(info.format.format)
        .setSamples(SampleCountFlagBits::e1)
        .setLoadOp(AttachmentLoadOp::eClear)
        .setStoreOp(AttachmentStoreOp::eStore)
        .setStencilLoadOp(AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(AttachmentStoreOp::eDontCare)
        .setInitialLayout(ImageLayout::eUndefined)
        .setFinalLayout(ImageLayout::ePresentSrcKHR);

    AttachmentDescription depthAttachment;
    depthAttachment
        .setFormat(depthTexture->getFormat())
        .setSamples(SampleCountFlagBits::e1)
        .setLoadOp(AttachmentLoadOp::eClear)
        .setStoreOp(AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(AttachmentStoreOp::eDontCare)
        .setInitialLayout(ImageLayout::eUndefined)
        .setFinalLayout(ImageLayout::ePresentSrcKHR);

    std::vector attachments = {colorAttachment, depthAttachment};
    createInfo.setAttachments(attachments);
    
    AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setLayout(ImageLayout::eColorAttachmentOptimal);

    AttachmentReference depthAttachmentRef;
    depthAttachmentRef
        .setLayout(ImageLayout::eDepthStencilAttachmentOptimal)
        .setAttachment(1);

    SubpassDescription subpass;
    subpass
        .setPipelineBindPoint(PipelineBindPoint::eGraphics)
        .setColorAttachments(colorAttachmentRef)
        .setPDepthStencilAttachment(&depthAttachmentRef);
    createInfo.setSubpasses(subpass);

    SubpassDependency dependency;
    dependency
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput | PipelineStageFlagBits::eLateFragmentTests)
        .setSrcAccessMask(AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput | PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstAccessMask(AccessFlagBits::eColorAttachmentWrite | AccessFlagBits::eDepthStencilAttachmentWrite);
    createInfo.setDependencies(dependency);

    renderPass_ = Device::getInstance()->getDevice().createRenderPass(createInfo);
}
}



