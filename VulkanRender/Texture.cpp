#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Device.h"
#include "CommandManager.h"
#include "stb_image.h"
#include "Swapchain.h"
#include "Memory/Memory.h"

namespace vkRender
{
US_VKN;
Texture* Texture::createFormFile(const std::string& fileName)
{
    Texture *texture = new (std::nothrow) Texture();
    if (texture && texture->init(fileName))
    {
        return texture;
    }
    return nullptr;
}

Texture* Texture::createDepth(uint32_t w, uint32_t h)
{
    Texture *texture = new (std::nothrow) Texture(w, h);
    if (texture && texture->init())
    {
        return texture;
    }
    return nullptr;
}

bool Texture::init(const std::string& fileName)
{
    if (!(data_ = stbi_load(fileName.c_str(), &width_, &height_, & TexChannels, STBI_rgb_alpha)))
    {
       return false;
    }

    format_ = Format::eR8G8B8A8Srgb;
    createImage(ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled);
    
    auto staging = Buffer::create(BufferUsageFlagBits::eTransferSrc, width_ * height_ * 4);
    staging->data(data_);

    transitionLayout(format_, ImageLayout::eUndefined, ImageLayout::eTransferDstOptimal);

    BufferImageCopy region;
    region
        .setImageOffset({0, 0, 0})
        .setImageExtent({static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1})
        .setImageSubresource({ImageAspectFlagBits::eColor, 0, 0, 1});
    
    CommandManager::Instance()->record([&](const CommandBuffer &cmd)
    {
        cmd.copyBufferToImage(staging->getBuffer(), texture_, ImageLayout::eTransferDstOptimal, region);
    });
    
    transitionLayout(format_, ImageLayout::eTransferDstOptimal, ImageLayout::eShaderReadOnlyOptimal);
    
    staging->release();

    textureView_ = Swapchain::newImageView(texture_, format_);

    createSampler();
    
    return true;
}

bool Texture::init()
{
    std::vector candidates = {Format::eD32Sfloat, Format::eD32SfloatS8Uint, Format::eD24UnormS8Uint};
    ImageTiling tiling = ImageTiling::eOptimal;
    FormatFeatureFlags feature = FormatFeatureFlagBits::eDepthStencilAttachment;
    
    for (Format format : candidates)
    {
        auto properties = Device::getInstance()->getGPU().getFormatProperties(format);

        if (tiling == ImageTiling::eLinear && (properties.optimalTilingFeatures & feature) == feature)
        {
            format_ = format;
        }
        else if (tiling == ImageTiling::eOptimal && (properties.optimalTilingFeatures & feature) == feature)
        {
            format_ = format;
        }
    }
    
    createImage(ImageUsageFlagBits::eDepthStencilAttachment);
    textureView_ = Swapchain::newImageView(texture_, format_, ImageAspectFlagBits::eDepth);

    transitionLayout(format_, ImageLayout::eUndefined, ImageLayout::eDepthStencilAttachmentOptimal);
    
    return true;
}

DescriptorImageInfo Texture::newDescriptor() const
{
    DescriptorImageInfo imageInfo;
    imageInfo
        .setImageLayout(ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(textureView_)
        .setSampler(textureSampler_);

    return imageInfo;
}

void Texture::transitionLayout(Format format, ImageLayout old, ImageLayout layout) const
{
    PipelineStageFlags sourceStage;
    PipelineStageFlags destinationStage;

    ImageMemoryBarrier barrier;
    ImageAspectFlags aspect;

    barrier
        .setImage(texture_)
        .setOldLayout(old)
        .setNewLayout(layout)
        .setDstQueueFamilyIndex(QueueFamilyIgnored)
        .setSrcQueueFamilyIndex(QueueFamilyIgnored);

    if (layout == ImageLayout::eDepthStencilAttachmentOptimal)
    {
        aspect = ImageAspectFlagBits::eDepth;
        if (format == Format::eD32SfloatS8Uint || format == Format::eD24UnormS8Uint)
        {
            aspect |= ImageAspectFlagBits::eStencil;
        }
    }
    else
    {
        aspect = ImageAspectFlagBits::eColor;
    }
    
    if (old == ImageLayout::eUndefined && layout == ImageLayout::eTransferDstOptimal)
    {
        barrier.setDstAccessMask(AccessFlagBits::eTransferWrite);

        sourceStage = PipelineStageFlagBits::eTopOfPipe;
        destinationStage = PipelineStageFlagBits::eTransfer;
    } else if (old == ImageLayout::eTransferDstOptimal && layout == ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier
            .setSrcAccessMask(AccessFlagBits::eTransferWrite)
            .setDstAccessMask(AccessFlagBits::eShaderRead);

        sourceStage = PipelineStageFlagBits::eTransfer;
        destinationStage = PipelineStageFlagBits::eFragmentShader;
    } else if (old == ImageLayout::eUndefined && layout == ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.setDstAccessMask(AccessFlagBits::eDepthStencilAttachmentRead |AccessFlagBits::eDepthStencilAttachmentWrite);

        sourceStage = PipelineStageFlagBits::eTopOfPipe;
        destinationStage = PipelineStageFlagBits::eEarlyFragmentTests;
    }

    barrier.setSubresourceRange({aspect, 0, 1, 0, 1});
    
    CommandManager::Instance()->record([&](const CommandBuffer &cmd)
    {
        cmd.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
    });
}

void Texture::createSampler()
{
    SamplerCreateInfo createInfo;
    
    createInfo
        .setMagFilter(Filter::eLinear)
        .setMinFilter(Filter::eLinear)
        .setAddressModeU(SamplerAddressMode::eRepeat)
        .setAddressModeV(SamplerAddressMode::eRepeat)
        .setAddressModeW(SamplerAddressMode::eRepeat)
        .setMaxAnisotropy(Device::getInstance()->properties.limits.maxSamplerAnisotropy)
        .setBorderColor(BorderColor::eIntOpaqueBlack)
        .setCompareOp(CompareOp::eAlways)
        .setMipmapMode(SamplerMipmapMode::eLinear);

    textureSampler_ = Device::getInstance()->getDevice().createSampler(createInfo);
}

void Texture::release() const
{
    stbi_image_free(data_);
    Device::getInstance()->getDevice().destroyImage(texture_);
    Device::getInstance()->getDevice().destroySampler(textureSampler_);
    Device::getInstance()->getDevice().destroyImageView(textureView_);
}

Texture::Texture(uint32_t w, uint32_t h):
width_(w), height_(h), format_(Format::eUndefined)
{
}

void Texture::createImage(ImageUsageFlags usage)
{
    ImageCreateInfo createInfo;
    createInfo
        .setImageType(ImageType::e2D)
        .setExtent({static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1})
        .setMipLevels(1)
        .setArrayLayers(1)
        .setFormat(format_)
        .setTiling(ImageTiling::eOptimal)
        .setInitialLayout(ImageLayout::eUndefined)
        .setUsage(usage)
        .setSharingMode(SharingMode::eExclusive)
        .setSamples(SampleCountFlagBits::e1);

    texture_ = Device::getInstance()->getDevice().createImage(createInfo);
    
    Memory::Binding(texture_, MemoryPropertyFlagBits::eDeviceLocal);
}

}
