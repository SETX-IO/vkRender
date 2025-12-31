#include "Context.h"

#include "CommandManager.h"
#include "Device.h"
#include "Memory/Memory.h"
#include "Memory/DebugUtil.h"

namespace vkRender
{
US_VKN;
std::unique_ptr<Context> Context::s_instance = nullptr;

void Context::Init(const std::vector<const char*>& extensions)
{
    if (!s_instance)
    {
        s_instance = std::make_unique<Context>();
        assert(s_instance->init(extensions), "Instacne Init Fialy.");
    }
}

Context* Context::getInstance()
{
    assert(s_instance, "Instance is nullptr.");
    return s_instance.get();
}

void Context::addReleaseObj(Release* obj)
{
    if (obj == nullptr)
    {
        return;
    }
    releasePool_.push_back(obj);
}

bool Context::init(const std::vector<const char*>& extensions)
{
    createVkInstance(extensions);
    debugUtil_ = std::make_unique<DebugUtil>(*DebugUtil::create(vkInstance_));
    
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
