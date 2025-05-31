#pragma once

#include "vkRender.h"

namespace vkRender
{
class Buffer
{
public:
    static Buffer* createDeviceLocal(vk::BufferUsageFlags bufferType, vk::DeviceSize size);
    static Buffer* create(vk::BufferUsageFlags bufferType, vk::DeviceSize size);

    bool init(vk::BufferUsageFlags bufferType, vk::MemoryPropertyFlags property);
    

    void copy(const Buffer &dstBuffer);
    
    void release();
    
    vk::DeviceMemory &getMemory() {return memory_;}
    vk::Buffer &getBuffer() {return buffer_;}
    uint32_t size() const {return size_;}
    
    operator vk::Buffer() const {return buffer_;}

    void *data_ = nullptr;
private:
    Buffer(vk::DeviceSize size);
    Buffer() = delete;
    ~Buffer();

    void createBuffer(vk::BufferUsageFlags bufferType);
    void allocateMemory(vk::MemoryPropertyFlags property);

    vk::DeviceSize size_;
    
    vk::Buffer buffer_;
    vk::DeviceMemory memory_;
};
}