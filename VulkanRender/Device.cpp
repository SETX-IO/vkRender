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
        properties = pDevice_.getProperties();
        result = true;
    }
    
    return result;
}

Semaphore Device::newSemaphore() const
{
    constexpr  SemaphoreCreateInfo createInfo;
    return device_.createSemaphore(createInfo);
}

Fence Device::newFence() const
{
    constexpr FenceCreateInfo createInfo{FenceCreateFlagBits::eSignaled};
    return device_.createFence(createInfo);
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
            pDevice_ = physicalDevice;
            std::cout << physicalDevice.getProperties().deviceName << "\n";

            indices_ = QueueFamilyIndices::find(pDevice_, surface);
            
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
    
    device_ = pDevice_.createDevice(createInfo);

    graphicsQueue = device_.getQueue(indices_.graphicsFamily.value(), 0);
    presentQueue = device_.getQueue(indices_.presentFamily.value(), 0);
}
}
