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
    
    void data(const void* data) const;
    void copy(const Buffer &dstBuffer) const;
    // void reSize(vk::DeviceSize size);
    
    void release() const;
    
    vk::Buffer &getBuffer() {return buffer_;}
    vk::DescriptorBufferInfo newDescriptor() const;
    vk::DeviceSize size() const {return size_;}
    
    // operator vk::Buffer() const {return buffer_;}

private:
    Buffer(vk::DeviceSize size);
    Buffer() = delete;

    void createBuffer(vk::BufferUsageFlags bufferType);

    vk::DeviceSize size_;
    vk::BufferUsageFlags type_;
    vk::MemoryPropertyFlags property_;
    
    vk::Buffer buffer_;
    // vk::DeviceMemory memory_;
    
    void *data_ = nullptr;
};
}