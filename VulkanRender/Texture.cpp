#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Device.h"
#include "Buffer.h"
#include "stb_image.h"
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

    // auto cmdBuf = CommandManager::Instance()->newCmdBuffers(1)[0];
    
    // ImageLayout older;
    // ImageLayout new_;
    //
    // ImageMemoryBarrier barrier;
    // barrier
    //     .setOldLayout(older)
    //     .setNewLayout(new_)
    //     .setDstQueueFamilyIndex(QueueFamilyIgnored)
    //     .setSrcQueueFamilyIndex(QueueFamilyIgnored)
    //     .setImage(texture_)
    //     .setSubresourceRange({ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    
    // cmdBuf.pipelineBarrier({}, {}, {}, {}, {}, barrier);
    
    return true;
}

void Texture::release()
{
    stbi_image_free(data_);
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
        .setUsage(ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eTransferSrc)
        .setSharingMode(SharingMode::eExclusive)
        .setSamples(SampleCountFlagBits::e1);

    texture_ = Device::getInstance()->getDevice().createImage(createInfo);

    MemoryRequirements memory = Device::getInstance()->getDevice().getImageMemoryRequirements(texture_);

    auto memory_ = Memory::AllocateMemory(MemoryPropertyFlagBits::eDeviceLocal,memory);

    Device::getInstance()->getDevice().bindImageMemory(texture_, memory_, 0);
}

bool Texture::allocateMemory()
{
    
    return true;
}
}
