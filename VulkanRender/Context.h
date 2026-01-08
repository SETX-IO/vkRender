#pragma once
#include "vulkan/vulkan.hpp"
#include "Memory/DebugUtil.h"

namespace vkRender
{
class DebugUtil;
    
class Context final
{
public:
    
    Context(const std::vector<const char*>& extensions, const std::string& EngineName = "", const std::string& AppName = "");
    
    // get Vulkan instance
    vk::Instance &getVkInstance() {return vkInstance_;}
    VkSurfaceKHR &getSurface() {return surface_;}
    std::vector<vk::PhysicalDevice> getPhysicalDevices() const;
    void release() const;
    
    ~Context() = default;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(const Context&&) = delete;
    
private:    
    vk::Instance vkInstance_;
    VkSurfaceKHR surface_ = {};
};
}
