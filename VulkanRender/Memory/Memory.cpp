#include "Memory.h"

#include "Device.h"

namespace vkRender
{
US_VKN;
uint64_t Memory::memoryOffset = 0;
uint16_t Memory::bufferCount = 0;
std::vector<DeviceMemory> Memory::memories_ = std::vector<DeviceMemory>(10);
std::map<MemoryPropertyFlags, DeviceMemory> Memory::memoryCache_ = {};

void* Memory::Binding(const Buffer& buffer, MemoryPropertyFlags property)
{
    auto requirements = Device::getInstance()->getDevice().getBufferMemoryRequirements(buffer);
    auto memory = AllocateMemory(property, requirements);
    auto offset = bufferCount * requirements.alignment;

    Device::getInstance()->getDevice().bindBufferMemory(buffer, memory, 0);
    
    if (property & MemoryPropertyFlagBits::eHostVisible)
    {
        return Device::getInstance()->getDevice().mapMemory(memory, 0, requirements.size);
    }

    return nullptr;
}

void Memory::Binding(const Image& image, MemoryPropertyFlags property)
{
    auto requirements = Device::getInstance()->getDevice().getImageMemoryRequirements(image);
    auto memory = AllocateMemory(property, requirements);
    auto offset = bufferCount * requirements.alignment;

    Device::getInstance()->getDevice().bindImageMemory(image, memory, 0);
}

DeviceMemory Memory::AllocateMemory(MemoryPropertyFlags property, const MemoryRequirements& requirements)
{
    // if (memoryCache_.find(property) != memoryCache_.end())
    // {
    //     return memoryCache_.at(property);
    // }
    PhysicalDeviceMemoryProperties physicalMemory = Device::getInstance()->getGPU().getMemoryProperties();
    const uint32_t typeFilter = requirements.memoryTypeBits;
    uint32_t typeIndex = 0;

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
    memories_[bufferCount++] = result;

    return result;
}

void Memory::release()
{
    for (auto memory : memories_)
    {
        Device::getInstance()->getDevice().freeMemory(memory);
    }
    for (auto memory : memoryCache_)
    {
        if (memory.first & MemoryPropertyFlagBits::eHostVisible)
        {
            Device::getInstance()->getDevice().unmapMemory(memory.second);
        }
        Device::getInstance()->getDevice().freeMemory(memory.second);
    }
}
}

