#include "Swapchain.h"

#include "Context.h"
#include "Device.h"
#include "Texture.h"
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
    Device::Instance()->getDevice().waitIdle();

    auto frameSize = Context::getInstance()->getFrameSize();
    if (frameSize == Extent2D{0, 0})
    {
        return;
    }
    
    releaseSwapchain();

    queryInfo();
    createSwapChain();
    createFramebuffer();
}

ImageView Swapchain::newImageView(const Image& image, Format format, ImageAspectFlags aspect, uint32_t mipLevels)
{
    ImageSubresourceRange subresourceRange(aspect, 0, mipLevels, 0, 1);
    
    ImageViewCreateInfo createInfo;
    createInfo
        .setImage(image)
        .setViewType(ImageViewType::e2D)
        .setFormat(format)
        .setSubresourceRange(subresourceRange);
    
    return Device::Instance()->getDevice().createImageView(createInfo);
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
    auto device = Device::Instance()->getDevice();
    
    releaseSwapchain();
    
    device.destroyRenderPass(renderPass_);
    depthTexture->release();
}

void Swapchain::releaseSwapchain() const
{
    auto device = Device::Instance()->getDevice();
    
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
    const auto& pDevice = Device::Instance()->getGPU();
    const auto& surface = Context::getInstance()->getSurface();
    
    auto capabilities = pDevice.getSurfaceCapabilitiesKHR(surface);
    auto Formats = pDevice.getSurfaceFormatsKHR(surface);
    auto presentModes = pDevice.getSurfacePresentModesKHR(surface);

    
    info.format = Formats[0];
    for (auto format : Formats)
    {
        if (format.format == Format::eB8G8R8A8Srgb && format.colorSpace == ColorSpaceKHR::eSrgbNonlinear)
        {
            info.format = format;
            break;
        }
    }

    info.presentMode = PresentModeKHR::eFifo;
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == PresentModeKHR::eMailbox)
        {
            info.presentMode = presentMode;
            break;
        }
    }

    info.transform = capabilities.currentTransform;
    info.imageCount = std::clamp(capabilities.minImageCount + MAX_FRAME_IN_FLIGHT - capabilities.minImageCount, capabilities.minImageCount, capabilities.maxImageCount);
    info.extent = Extent2D(
        std::clamp(Context::getInstance()->getFrameSize().width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(Context::getInstance()->getFrameSize().height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        );
}

void Swapchain::createSwapChain()
{
    const auto device = Device::Instance();
    
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
    auto device = Device::Instance();

    std::vector images = device->getDevice().getSwapchainImagesKHR(swapchain_);
    
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

    renderPass_ = Device::Instance()->getDevice().createRenderPass(createInfo);
}
}



