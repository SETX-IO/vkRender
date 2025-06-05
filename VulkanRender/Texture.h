#pragma once

#include "Buffer.h"
#include "vkRender.h"

namespace vkRender
{
class Texture
{
public:
    static Texture *createFormFile(const std::string &fileName);

    bool init(const std::string& fileName);

    vk::DescriptorImageInfo newDescriptor() const;
    
    void release() const;
private:
    int width_ = 0;
    int height_ = 0;
    int TexChannels = 0;
    
    uByte *data_ = nullptr;
    
    vk::Image texture_;
    vk::DeviceMemory memory_;
    vk::ImageView textureView_;
    vk::Sampler textureSampler_;

    void createImage();
    void transitionLayout(vk::Format format, vk::ImageLayout old, vk::ImageLayout layout) const;
    void createSampler();
};
}
