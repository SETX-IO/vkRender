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

bool Texture::init(const std::string& fileName)
{
    if (!(data_ = stbi_load(fileName.c_str(), &width_, &height_, & TexChannels, STBI_rgb_alpha)))
    {
       return false;
    }

    createImage();
    
    auto staging = Buffer::create(BufferUsageFlagBits::eTransferSrc, width_ * height_ * 4);
    staging->data(data_);

    transitionLayout(Format::eR8G8B8A8Srgb, ImageLayout::eUndefined, ImageLayout::eTransferDstOptimal);

    BufferImageCopy region;
    region
        .setImageOffset({0, 0, 0})
        .setImageExtent({static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1})
        .setImageSubresource({ImageAspectFlagBits::eColor, 0, 0, 1});
    
    CommandManager::Instance()->record([&](const CommandBuffer &cmd)
    {
        cmd.copyBufferToImage(staging->getBuffer(), texture_, ImageLayout::eTransferDstOptimal, region);
    });
    
    transitionLayout(Format::eR8G8B8A8Srgb, ImageLayout::eTransferDstOptimal, ImageLayout::eShaderReadOnlyOptimal);
    
    staging->release();

    textureView_ = Swapchain::newImageView(texture_, Format::eR8G8B8A8Srgb);

    createSampler();
    
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
    barrier
        .setImage(texture_)
        .setOldLayout(old)
        .setNewLayout(layout)
        .setDstQueueFamilyIndex(QueueFamilyIgnored)
        .setSrcQueueFamilyIndex(QueueFamilyIgnored)
        .setSubresourceRange({ImageAspectFlagBits::eColor, 0, 1, 0, 1});

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
    }
    
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
    Device::getInstance()->getDevice().freeMemory(memory_);
    Device::getInstance()->getDevice().destroySampler(textureSampler_);
    Device::getInstance()->getDevice().destroyImageView(textureView_);
}

void Texture::createImage()
{
    ImageCreateInfo createInfo;
    createInfo
        .setImageType(ImageType::e2D)
        .setExtent({static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1})
        .setMipLevels(1)
        .setArrayLayers(1)
        .setFormat(Format::eR8G8B8A8Srgb)
        .setTiling(ImageTiling::eOptimal)
        .setInitialLayout(ImageLayout::eUndefined)
        .setUsage(ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled)
        .setSharingMode(SharingMode::eExclusive)
        .setSamples(SampleCountFlagBits::e1);

    texture_ = Device::getInstance()->getDevice().createImage(createInfo);

    MemoryRequirements memory = Device::getInstance()->getDevice().getImageMemoryRequirements(texture_);

    memory_ = Memory::AllocateMemory(MemoryPropertyFlagBits::eDeviceLocal, memory);

    Device::getInstance()->getDevice().bindImageMemory(texture_, memory_, 0);
}

}
