#pragma once
#include "glm/glm.hpp"

#include "vkRender.h"

namespace vkRender
{
constexpr int MAX_FRAME_IN_FLIGHT = 3;
static auto empty = std::vector<const char*>();

using CreateSurfacerFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

struct UniformObj
{
    glm::mat4 module;
    glm::mat4 view;
    glm::mat4 proj;
};

class Context final
{
public:
    // get Context Instance. parameters use on initialize
    static Context* getInstance(const std::vector<const char*> &extensions = empty, const CreateSurfacerFunc& func = nullptr);

    Context() = default;
    ~Context();
    
    // get Vulkan instance
    vk::Instance & getVkInstance() {return vkInstance;}
    vk::SurfaceKHR &getSurface() {return surface;}
    
    bool init(const std::vector<const char*>& extensions, const CreateSurfacerFunc& func);
    
    void createVkInstance(const std::vector<const char*>& extensions);
    
    static Context* s_instance;

private:
    vk::Instance vkInstance;
    
    vk::SurfaceKHR surface;
};
}
