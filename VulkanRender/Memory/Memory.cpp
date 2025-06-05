#include "Memory.h"

#include "Device.h"

namespace vkRender
{
US_VKN;
uint64_t Memory::memoryOffset = 0;
uint16_t Memory::bufferCount = 0;
std::map<MemoryPropertyFlags, DeviceMemory> Memory::memoryCache_ = {};

void* Memory::BindBuffer(const Buffer& buffer, MemoryPropertyFlags property)
{
    auto requirements = Device::getInstance()->getDevice().getBufferMemoryRequirements(buffer);
    auto memory = AllocateMemory(property, requirements);
    auto offset = bufferCount * requirements.alignment;

    Device::getInstance()->getDevice().bindBufferMemory(buffer, memory, offset);
    
    if (property & MemoryPropertyFlagBits::eHostVisible)
    {
        return Device::getInstance()->getDevice().mapMemory(memory, offset, requirements.size);
    }

    return nullptr;
}

DeviceMemory Memory::AllocateMemory(MemoryPropertyFlags property, const MemoryRequirements& requirements)
{
    // if (memoryCache_.find(property) != memoryCache_.end())
    // {
    //     return memoryCache_.at(property);
    // }
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
        // .setAllocationSize(MAX_SIZE)
        .setAllocationSize(requirements.size)
        .setMemoryTypeIndex(typeIndex);

    auto result = Device::getInstance()->getDevice().allocateMemory(allocateInfo);
    // memoryCache_[property] = result;

    return result;
}

void Memory::release()
{
    for (auto memory : memoryCache_)
    {
        Device::getInstance()->getDevice().unmapMemory(memory.second);
        Device::getInstance()->getDevice().freeMemory(memory.second);
    }
}
}

