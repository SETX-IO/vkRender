#pragma once

#include "vkRender.h"

namespace vkRender
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    operator bool() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    bool equal() const {return graphicsFamily.value() == presentFamily.value();}

    static QueueFamilyIndices find(const vk::PhysicalDevice &pDevice, const vk::SurfaceKHR &surface)
    {
        QueueFamilyIndices familyIndices;
        std::vector familyProperties = pDevice.getQueueFamilyProperties();

        for (int i = 0; i < familyProperties.size(); ++i)
        {
            const auto& familyProperty = familyProperties[i];
            if (familyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                familyIndices.graphicsFamily = i;
            }

            if (pDevice.getSurfaceSupportKHR(i, surface))
            {
                familyIndices.presentFamily = i;
            }

            if (familyIndices)
            {
                break;
            }
        }

        return familyIndices;
    }
};

class Device
{
public:
    ~Device();
    static Device *getInstance();
    
    bool init();

    void release();

    vk::PhysicalDevice &getGPU() {return GPU_;}
    vk::Device &getDevice() {return device_;}

    vk::Semaphore &newSemaphore();
    vk::Fence &newFence();

    QueueFamilyIndices indices_;
    
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::PhysicalDeviceProperties properties;
private:
    bool pickPhysicalDevice();
    void createLogicalDevice();
    
    vk::PhysicalDevice GPU_;
    vk::Device device_;

    std::stack<vk::Semaphore> semaphoreStack_;
    std::stack<vk::Fence> fenceStack_;
    
    static Device *s_instance;
};
}
