#pragma once
#include "glm/glm.hpp"

#include "vkRender.h"
#include "Memory/DebugUtil.h"

namespace vkRender
{
static auto empty = std::vector<const char*>();

class Context final
{
public:
    // get Context Instance. parameters use on initialize
    static Context* getInstance(const std::vector<const char*> &extensions = empty);
    
    // get Vulkan instance
    vk::Instance & getVkInstance() {return vkInstance_;}
    VkSurfaceKHR &getSurface() {return surface_;}

    void setFrameSize(uint32_t width, uint32_t height) { frameSize_ = vk::Extent2D(width, height); }
    const vk::Extent2D& getFrameSize() const {return frameSize_;}
    
    bool init(const std::vector<const char*>& extensions);
    void release() const;

    Context() = default;
    ~Context();
private:
    vk::Instance vkInstance_;
    VkSurfaceKHR surface_ = {};

    DebugUtil *debugUtil_ = nullptr;

    vk::Extent2D frameSize_ = {};
    
    static Context* s_instance;
    
    void createVkInstance(const std::vector<const char*>& extensions);
};
}
