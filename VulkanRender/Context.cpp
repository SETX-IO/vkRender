#include "Context.h"

#include "CommandManager.h"
#include "Device.h"
#include "Memory/Memory.h"

namespace vkRender
{
US_VKN;
Context::Context(const std::vector<const char*>& extensions, const std::string& EngineName, const std::string& AppName)
{
    ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_4);
    appInfo.setPEngineName(EngineName.c_str());
    appInfo.setPApplicationName(AppName.c_str());
    
    InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);
    
#if _DEBUG
    const std::vector enableLayers = { "VK_LAYER_KHRONOS_validation"};
    createInfo.setPEnabledLayerNames(enableLayers);
#endif
    
    if (!extensions.empty())
    {
        createInfo.setPEnabledExtensionNames(extensions);
    }

    vkInstance_ = createInstance(createInfo);
}

std::vector<PhysicalDevice> Context::getPhysicalDevices() const
{
    std::vector devices{ vkInstance_.enumeratePhysicalDevices() };
    
    return devices;
}

void Context::release() const
{
    Memory::release();
    Device::Instance()->release();
    
    vkInstance_.destroySurfaceKHR(surface_);
    // debugUtil_->destroy();
    vkInstance_.destroy();
}
}
