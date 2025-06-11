#include "Device.h"

#include "Context.h"

namespace vkRender
{
US_VKN;
Device *Device::s_instance = nullptr;

Device::~Device()
{
    device_.destroy();
    s_instance = nullptr;
}

Device* Device::getInstance()
{
    if (!s_instance)
    {
        s_instance = new (std::nothrow) Device();
        s_instance->init();
    }

    return s_instance;
}

bool Device::init()
{
    bool result = false;
    if (pickPhysicalDevice())
    {
        createLogicalDevice();
        properties = GPU_.getProperties();
        result = true;
    }
    
    return result;
}

void Device::release()
{
    int semaphoreCount = semaphoreStack_.size();
    for (int i = 0; i < semaphoreCount; ++i)
    {
        auto semaphore = semaphoreStack_.top();
        device_.destroySemaphore(semaphore);
        semaphoreStack_.pop();
    }

    int fenceCount = fenceStack_.size();
    for (int i = 0; i < fenceCount; ++i)
    {
        auto fence = fenceStack_.top();
        device_.destroyFence(fence);
        fenceStack_.pop();
    }
    device_.destroy();
}

Semaphore &Device::newSemaphore()
{
    constexpr  SemaphoreCreateInfo createInfo;
    auto semaphore = device_.createSemaphore(createInfo);
    semaphoreStack_.push(semaphore);
    
    return semaphoreStack_.top();
}

Fence &Device::newFence()
{
    constexpr FenceCreateInfo createInfo{FenceCreateFlagBits::eSignaled};
    auto fence = device_.createFence(createInfo);
    fenceStack_.push(fence);
    
    return fenceStack_.top();
}

bool Device::pickPhysicalDevice()
{
    std::vector deviceExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    std::vector physicalDevices = Context::getInstance()->getVkInstance().enumeratePhysicalDevices();
    
    std::set<std::string> requiredExtensions(deviceExtension.begin(), deviceExtension.end());
    auto surface = Context::getInstance()->getSurface();
    
    for (const auto& physicalDevice : physicalDevices)
    {
        std::vector extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        auto format = physicalDevice.getSurfaceFormatsKHR(surface);
        auto presentMode = physicalDevice.getSurfacePresentModesKHR(surface);
        auto feature = physicalDevice.getFeatures();
        
        for (const auto & extensionProperty : extensionProperties)
        {
            requiredExtensions.erase(extensionProperty.extensionName);
        }
        
        if (physicalDevice.getFeatures().geometryShader && requiredExtensions.empty()
            && !format.empty() && !presentMode.empty() && feature.samplerAnisotropy)
        {
            GPU_ = physicalDevice;
            std::cout << physicalDevice.getProperties().deviceName << "\n";

            indices_ = QueueFamilyIndices::find(GPU_, surface);
            
            return true;
        }
    }
    
    return false;
}

void Device::createLogicalDevice()
{
    float queuePriority = 1.0f;
    DeviceCreateInfo createInfo;
    std::vector deviceExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    std::vector<DeviceQueueCreateInfo> queueCreateInfos;
    std::set uniqueQueueFamilies = {indices_.graphicsFamily.value(), indices_.presentFamily.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo
            .setQueueFamilyIndex(queueFamily)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    createInfo
        .setPEnabledExtensionNames(deviceExtension)
        .setQueueCreateInfos(queueCreateInfos);
    
    device_ = GPU_.createDevice(createInfo);

    graphicsQueue = device_.getQueue(indices_.graphicsFamily.value(), 0);
    presentQueue = device_.getQueue(indices_.presentFamily.value(), 0);
}
}
