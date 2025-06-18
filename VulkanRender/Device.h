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

    static QueueFamilyIndices find(const vk::PhysicalDevice &gpu, const vk::SurfaceKHR &surface)
    {
        QueueFamilyIndices familyIndices;
        std::vector familyProperties = gpu.getQueueFamilyProperties();

        for (int i = 0; i < familyProperties.size(); ++i)
        {
            const auto& familyProperty = familyProperties[i];
            if (familyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                familyIndices.graphicsFamily = i;
            }

            if (gpu.getSurfaceSupportKHR(i, surface))
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
    static Device *Instance();
    
    bool init();

    void release();

    const vk::PhysicalDevice &getGPU() const {return GPU_;}
    const vk::Device &getDevice() const {return device_;}
    vk::PipelineCache &getPipelineCache() {return pipelineCache_;}

    vk::Semaphore &newSemaphore();
    vk::Fence &newFence();
    vk::Queue &getQueue(uint32_t family);

    QueueFamilyIndices indices_;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::PhysicalDeviceProperties properties;
private:
    bool pickPhysicalDevice();
    void createLogicalDevice();
    void createPipelineCache();
    
    vk::PhysicalDevice GPU_;
    vk::Device device_;

    vk::PipelineCache pipelineCache_;

    std::stack<vk::Semaphore> semaphoreStack_;
    std::stack<vk::Fence> fenceStack_;

    std::map<uint32_t, vk::Queue> queueCache_;
    
    static Device *s_instance;
};
}
