#pragma once
#include <map>
#include "vkRender.h"

namespace vkRender
{
class Memory
{
public:
    void Binding(const vk::Buffer &buffer, vk::MemoryPropertyFlags property);
    
    static vk::DeviceMemory AllocateMemory(vk::MemoryPropertyFlags property, vk::MemoryRequirements requirements);
private:
    std::map<vk::MemoryPropertyFlags, vk::DeviceMemory> memoryCache_;

    static uint64_t memoryOffset;
};
}
