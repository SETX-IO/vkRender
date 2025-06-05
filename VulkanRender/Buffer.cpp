#include "Buffer.h"

#include "CommandManager.h"
#include "Context.h"
#include "Device.h"
#include "Memory/Memory.h"

namespace vkRender
{
US_VKN;

Buffer* Buffer::createDeviceLocal(BufferUsageFlags bufferType, DeviceSize size)
{
    Buffer *buffer = new (std::nothrow) Buffer(size);
    MemoryPropertyFlags property = MemoryPropertyFlagBits::eDeviceLocal;
    
    if (buffer && buffer->init(bufferType, property))
    {
        return buffer;
    }
    return nullptr;
}

Buffer* Buffer::create(BufferUsageFlags bufferType, DeviceSize size)
{
    Buffer *buffer = new (std::nothrow) Buffer(size);
    MemoryPropertyFlags property = MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent;
    if (buffer && buffer->init(bufferType, property))
    {
        return buffer;
    }
    return nullptr;
}

bool Buffer::init(BufferUsageFlags bufferType, MemoryPropertyFlags property)
{
    createBuffer(bufferType);
    
    MemoryRequirements memory = Device::getInstance()->getDevice().getBufferMemoryRequirements(buffer_);
    memory_ = Memory::AllocateMemory(property, memory);
    
    Device::getInstance()->getDevice().bindBufferMemory(buffer_, memory_, 0);
    
    if (property & MemoryPropertyFlagBits::eHostVisible)
    {
        data_ = Device::getInstance()->getDevice().mapMemory(memory_, 0, size_);
    }

    // data_ = Memory::BindBuffer(buffer_, property);
    
    return true;
}

void Buffer::release() const
{
    Device::getInstance()->getDevice().unmapMemory(memory_);
    
    Device::getInstance()->getDevice().destroyBuffer(buffer_);
    Device::getInstance()->getDevice().freeMemory(memory_);
}

void Buffer::copy(const Buffer& dstBuffer) const
{
    BufferCopy bufferCopy;
    bufferCopy
        .setSrcOffset(0)
        .setDstOffset(0)
        .setSize(dstBuffer.size());
    
    CommandManager::Instance()->record([&](const CommandBuffer &cmd)
    {
        cmd.copyBuffer(buffer_, dstBuffer, 1, &bufferCopy);
    });

    release();
}

void Buffer::data(const void* data) const
{
    memcpy(data_, data, size_);
}

Buffer::Buffer(DeviceSize size):
size_(size)
{}

void Buffer::createBuffer(BufferUsageFlags bufferType)
{
    BufferCreateInfo createInfo;
    createInfo
        .setSize(size_)
        .setUsage(bufferType)
        .setSharingMode(SharingMode::eExclusive);
    buffer_ = Device::getInstance()->getDevice().createBuffer(createInfo);
}
}
