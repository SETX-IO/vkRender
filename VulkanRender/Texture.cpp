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

    mipLevels_ = static_cast<uint32_t> (std::floor(std::log2(std::max(width_, height_))));
    format_ = Format::eR8G8B8A8Srgb;
    createImage(ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled);
    textureView_ = Swapchain::newImageView(texture_, format_, ImageAspectFlagBits::eColor, mipLevels_);
    
    auto staging = Buffer::create(BufferUsageFlagBits::eTransferSrc, width_ * height_ * 4);
    staging->data(data_);
    
    BufferImageCopy region;
    region
        .setImageOffset(0)
        .setImageExtent({static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1})
        .setImageSubresource({ImageAspectFlagBits::eColor, 0, 0, 1});

    transitionLayout(format_, ImageLayout::eUndefined, ImageLayout::eTransferDstOptimal);
    
    CommandManager::Instance()->record([&](const CommandBuffer &cmd)
    {
        cmd.copyBufferToImage(staging->getBuffer(), texture_, ImageLayout::eTransferDstOptimal, region);
    });
    
    transitionLayout(format_, ImageLayout::eTransferDstOptimal, ImageLayout::eShaderReadOnlyOptimal);
    // generateMipmaps();
    staging->release();

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
        auto properties = Device::Instance()->getGPU().getFormatProperties(format);

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

    barrier.setSubresourceRange({aspect, 0, mipLevels_, 0, 1});
    
    CommandManager::Instance()->record([&](const CommandBuffer &cmd)
    {
        cmd.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
    });
}

void Texture::generateMipmaps()
{
    ImageMemoryBarrier barrier;
    constexpr ImageSubresourceRange Range {ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    ImageSubresourceLayers Layers = {ImageAspectFlagBits::eColor, 1, 0, 1};
    int mipWidth = width_;
    int mipHeight = height_;
    
    barrier
        .setImage(texture_)
        .setSrcQueueFamilyIndex(QueueFamilyIgnored)
        .setDstQueueFamilyIndex(QueueFamilyIgnored)
        .setSubresourceRange(Range)
        .setOldLayout(ImageLayout::eTransferDstOptimal)
        .setNewLayout(ImageLayout::eTransferSrcOptimal)
        .setSrcAccessMask(AccessFlagBits::eTransferWrite)
        .setDstAccessMask(AccessFlagBits::eTransferRead);

    ImageBlit blit;
    blit.setSrcOffsets({0, {mipWidth, mipHeight, 1}})
        .setSrcSubresource(Layers)
        .setDstSubresource(Layers);

    CommandManager::Instance()->record([&](const CommandBuffer& cmd)
    {
        for (uint32_t i = 1; i < mipLevels_; ++i)
        {
            barrier.subresourceRange.setBaseMipLevel(i - 1);
            blit.srcSubresource.setMipLevel(i - 1);
            blit.setDstOffsets({0, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}})
                .dstSubresource.setMipLevel(i);

            cmd.pipelineBarrier(PipelineStageFlagBits::eTransfer, PipelineStageFlagBits::eTransfer,
                {}, {}, {}, barrier);
                
            cmd.blitImage(texture_, ImageLayout::eTransferSrcOptimal,
                texture_, ImageLayout::eTransferDstOptimal,
                blit, Filter::eLinear);

            barrier
                .setOldLayout(ImageLayout::eTransferSrcOptimal)
                .setNewLayout(ImageLayout::eShaderReadOnlyOptimal)
                .setSrcAccessMask(AccessFlagBits::eTransferRead)
                .setDstAccessMask(AccessFlagBits::eShaderRead);
                    
            cmd.pipelineBarrier(PipelineStageFlagBits::eTransfer, PipelineStageFlagBits::eFragmentShader,
        {}, {}, {}, barrier);
        }

        barrier
            .setOldLayout(ImageLayout::eTransferDstOptimal)
            .setNewLayout(ImageLayout::eShaderReadOnlyOptimal)
            .setSrcAccessMask(AccessFlagBits::eTransferRead)
            .setDstAccessMask(AccessFlagBits::eShaderRead)
            .subresourceRange.setBaseMipLevel(mipLevels_ - 1);
                            
        cmd.pipelineBarrier(PipelineStageFlagBits::eTransfer, PipelineStageFlagBits::eFragmentShader,
        {}, {}, {}, barrier);
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
        .setMaxAnisotropy(Device::Instance()->properties.limits.maxSamplerAnisotropy)
        .setBorderColor(BorderColor::eIntOpaqueBlack)
        .setCompareOp(CompareOp::eAlways)
        .setMipmapMode(SamplerMipmapMode::eLinear);

    textureSampler_ = Device::Instance()->getDevice().createSampler(createInfo);
}

void Texture::release() const
{
    stbi_image_free(data_);
    Device::Instance()->getDevice().destroyImage(texture_);
    Device::Instance()->getDevice().destroySampler(textureSampler_);
    Device::Instance()->getDevice().destroyImageView(textureView_);
}

Texture::Texture(uint32_t w, uint32_t h):
width_(w), height_(h), format_(Format::eUndefined), mipLevels_(1)
{
}

void Texture::createImage(ImageUsageFlags usage)
{
    ImageCreateInfo createInfo;
    createInfo
        .setImageType(ImageType::e2D)
        .setExtent({static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1})
        .setMipLevels(mipLevels_)
        .setArrayLayers(1)
        .setFormat(format_)
        .setTiling(ImageTiling::eOptimal)
        .setInitialLayout(ImageLayout::eUndefined)
        .setUsage(usage)
        .setSharingMode(SharingMode::eExclusive)
        .setSamples(SampleCountFlagBits::e1);

    texture_ = Device::Instance()->getDevice().createImage(createInfo);
    
    Memory::Binding(texture_, MemoryPropertyFlagBits::eDeviceLocal);
}

}
