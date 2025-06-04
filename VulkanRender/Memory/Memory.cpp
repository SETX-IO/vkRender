#include "Memory.h"

#include "Device.h"

namespace vkRender
{
US_VKN;
uint64_t Memory::memoryOffset = 0;

void Memory::Binding(const vk::Buffer& buffer, vk::MemoryPropertyFlags property)
{
    
}

DeviceMemory Memory::AllocateMemory(MemoryPropertyFlags property, MemoryRequirements requirements)
{
    PhysicalDeviceMemoryProperties physicalMemory = Device::getInstance()->getPDevice().getMemoryProperties();
    uint32_t typeFilter = requirements.memoryTypeBits;
    int typeIndex = 0;

    for (uint32_t i = 0; i < physicalMemory.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i) && (physicalMemory.memoryTypes[i].propertyFlags & property) == property)
        {
            typeIndex = i;
            break;
        }
    }
    MemoryAllocateInfo allocateInfo;
    allocateInfo
        .setAllocationSize(requirements.size)
        .setMemoryTypeIndex(typeIndex);

    auto result = Device::getInstance()->getDevice().allocateMemory(allocateInfo);

    return result;
}
}

