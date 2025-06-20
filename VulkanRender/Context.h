#pragma once
#include "glm/glm.hpp"

#include "vkRender.h"

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
    
    bool init(const std::vector<const char*>& extensions);
    void release() const;

    Context() = default;
    ~Context();
private:
    vk::Instance vkInstance_;
    VkSurfaceKHR surface_ = {};
    
    static Context* s_instance;
    
    void createVkInstance(const std::vector<const char*>& extensions);
};
}
