#pragma once
#include "Release.h"

#include "vulkan/vulkan.hpp"

namespace vkRender
{
class DebugUtil;

class Context final
{
public:
    
    static void Init(const std::vector<const char*> &extensions);
    
    // get Context Instance.
    static Context* getInstance();
    
    // get Vulkan instance
    vk::Instance &getVkInstance() {return vkInstance_;}
    VkSurfaceKHR &getSurface() {return surface_;}

    void addReleaseObj(Release *obj);
    void setFrameSize(uint32_t width, uint32_t height) { frameSize_ = vk::Extent2D(width, height); }
    const vk::Extent2D& getFrameSize() const {return frameSize_;}
    
    bool init(const std::vector<const char*>& extensions);
    void release() const;

    Context() = default;
    ~Context() = default;

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&&) = delete;
    
private:    
    vk::Instance vkInstance_;
    VkSurfaceKHR surface_ = {};

    std::vector<Release *> releasePool_;

    std::unique_ptr<DebugUtil> debugUtil_;

    vk::Extent2D frameSize_ = {};
    
    static std::unique_ptr<Context> s_instance;
    // static Context* s_instance;
    
    void createVkInstance(const std::vector<const char*>& extensions);
};
}
