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
    auto requirements = Device::Instance()->getDevice().getBufferMemoryRequirements(buffer);
    auto memory = AllocateMemory(property, requirements);
    auto offset = bufferCount * requirements.alignment;

    Device::Instance()->getDevice().bindBufferMemory(buffer, memory, 0);
    
    if (property & MemoryPropertyFlagBits::eHostVisible)
    {
        return Device::Instance()->getDevice().mapMemory(memory, 0, requirements.size);
    }

    return nullptr;
}

void Memory::Binding(const Image& image, MemoryPropertyFlags property)
{
    auto requirements = Device::Instance()->getDevice().getImageMemoryRequirements(image);
    auto memory = AllocateMemory(property, requirements);
    auto offset = bufferCount * requirements.alignment;

    Device::Instance()->getDevice().bindImageMemory(image, memory, 0);
}

DeviceMemory Memory::AllocateMemory(MemoryPropertyFlags property, const MemoryRequirements& requirements)
{
    // if (memoryCache_.find(property) != memoryCache_.end())
    // {
    //     return memoryCache_.at(property);
    // }
    PhysicalDeviceMemoryProperties physicalMemory = Device::Instance()->getGPU().getMemoryProperties();
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

    auto result = Device::Instance()->getDevice().allocateMemory(allocateInfo);

    memories_.push_back(result);
    ++bufferCount;
    
    return result;
}

void Memory::release()
{
    for (auto memory : memories_)
    {
        Device::Instance()->getDevice().freeMemory(memory);
    }
    for (auto memory : memoryCache_)
    {
        if (memory.first & MemoryPropertyFlagBits::eHostVisible)
        {
            Device::Instance()->getDevice().unmapMemory(memory.second);
        }
        Device::Instance()->getDevice().freeMemory(memory.second);
    }
}
}

