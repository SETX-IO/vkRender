#pragma once

#include "Buffer.h"
#include "vkRender.h"

namespace vkRender
{
class Texture
{
public:
    static Texture *createFormFile(const std::string &fileName);
    static Texture *createDepth(uint32_t w, uint32_t h);

    bool init(const std::string& fileName);
    bool init();

    vk::DescriptorImageInfo newDescriptor() const;
    vk::Format getFormat() const {return format_;}
    
    void release() const;
    const vk::ImageView &getView() const {return textureView_;}

    Texture() = default;
    Texture(uint32_t w, uint32_t h);
private:
    int width_ = 0;
    int height_ = 0;
    int TexChannels = 0;
    
    uByte *data_ = nullptr;

    vk::Format format_;
    vk::Image texture_;
    vk::DeviceMemory memory_;
    vk::ImageView textureView_;
    vk::Sampler textureSampler_;

    void createImage(vk::ImageUsageFlags usage);
    void transitionLayout(vk::Format format, vk::ImageLayout old, vk::ImageLayout layout) const;
    void createSampler();
};
}
