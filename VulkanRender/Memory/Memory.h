#pragma once
#include <map>
#include "vkRender.h"

namespace vkRender
{
class Memory
{
public:
    static void* Binding(const vk::Buffer &buffer, vk::MemoryPropertyFlags property);
    static void Binding(const vk::Image &image, vk::MemoryPropertyFlags property);
    
    static vk::DeviceMemory AllocateMemory(vk::MemoryPropertyFlags property, const vk::MemoryRequirements& requirements);\

    static void release();
private:
    static std::map<vk::MemoryPropertyFlags, vk::DeviceMemory> memoryCache_;
    static std::vector<vk::DeviceMemory> memories_;
    
    static uint64_t memoryOffset;
    static uint16_t bufferCount;

    static constexpr uint32_t MAX_SIZE = 1024 * 2024;
};
}
