#pragma once

#include "vkRender.h"
#include "QueueFamilyIndices.h"

namespace vkRender
{

struct QueueFamilyIndices;

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
