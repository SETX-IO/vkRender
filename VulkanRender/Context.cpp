#include "Context.h"

#include "CommandManager.h"
#include "Device.h"
#include "Memory/Memory.h"

namespace vkRender
{
US_VKN;
Context* Context::s_instance = nullptr;

Context* Context::getInstance(const std::vector<const char*>& extensions, const CreateSurfacerFunc& func)
{
    if (!s_instance)
    {
        s_instance = new(std::nothrow) Context();
        s_instance->init(extensions, func);
    }

    return s_instance;
}

Context::~Context()
{
    s_instance = nullptr;
}

bool Context::init(const std::vector<const char*>& extensions, const CreateSurfacerFunc& func)
{
    bool result = false;
    // create Vulkan Instance
    createVkInstance(extensions);

    if (func != nullptr)
    {
        surface = func(vkInstance);
    }
    
    return result;
}

void Context::release() const
{
    Memory::release();
    Device::getInstance()->release();
    
    vkInstance.destroySurfaceKHR(surface);
    vkInstance.destroy();
}


void Context::createVkInstance(const std::vector<const char*>& extensions)
{
    ApplicationInfo appInfo;
    appInfo
        .setApiVersion(VK_API_VERSION_1_4)
        .setPApplicationName("VK Render")
        .setPEngineName("Cocos2dx");

    const std::vector enableLayers = { "VK_LAYER_KHRONOS_validation" };
    InstanceCreateInfo instanceCreateInfo;

    instanceCreateInfo
        .setPEnabledLayerNames(enableLayers)
        .setPApplicationInfo(&appInfo);
    
    if (!extensions.empty())
    {
        instanceCreateInfo.setPEnabledExtensionNames(extensions);
    }

    vkInstance = createInstance(instanceCreateInfo);
}

}
