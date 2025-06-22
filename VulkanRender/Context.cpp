#include "Context.h"

#include "CommandManager.h"
#include "Device.h"
#include "Memory/Memory.h"

namespace vkRender
{
US_VKN;
Context* Context::s_instance = nullptr;

Context* Context::getInstance(const std::vector<const char*>& extensions)
{
    if (!s_instance)
    {
        s_instance = new(std::nothrow) Context();
        s_instance->init(extensions);
    }

    return s_instance;
}

void Context::addReleaseObj(Release* obj)
{
    if (obj == nullptr)
    {
        return;
    }
    releasePool_.push_back(obj);
}

Context::~Context()
{
    s_instance = nullptr;
}

bool Context::init(const std::vector<const char*>& extensions)
{
    createVkInstance(extensions);
    debugUtil_ = DebugUtil::create(vkInstance_);
    
    return true;
}

void Context::release() const
{
    for (auto release : releasePool_)
    {
        release->release();
        release = nullptr;
    }
    Memory::release();
    Device::Instance()->release();
    
    vkInstance_.destroySurfaceKHR(surface_);
    debugUtil_->destroy();
    vkInstance_.destroy();
}

void Context::createVkInstance(const std::vector<const char*>& extensions)
{
    InstanceCreateInfo createInfo;
    ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_4);
    createInfo.setPApplicationInfo(&appInfo);

    const std::vector enableLayers = { "VK_LAYER_KHRONOS_validation"};
    createInfo.setPEnabledLayerNames(enableLayers);
    
    if (!extensions.empty())
    {
        createInfo.setPEnabledExtensionNames(extensions);
    }

    vkInstance_ = createInstance(createInfo);
}

}
