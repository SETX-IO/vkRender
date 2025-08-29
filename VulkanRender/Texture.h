#pragma once
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
    const vk::ImageView &getView() const {return textureView_;}
    
    void release() const;

    Texture() = default;
    Texture(uint32_t w, uint32_t h);
    virtual ~Texture() = default;
private:
    int width_ = 0;
    int height_ = 0;
    int TexChannels = 0;
    
    uByte *data_ = nullptr;

    uint32_t mipLevels_;
    
    vk::Format format_;
    vk::Image texture_;
    vk::ImageView textureView_;
    vk::Sampler textureSampler_;

    void createImage(vk::ImageUsageFlags usage);
    void transitionLayout(vk::Format format, vk::ImageLayout old, vk::ImageLayout layout) const;
    void generateMipmaps();
    void createSampler();
};
}
