#include "Buffer.h"

#include "Context.h"
#include "Device.h"

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
    allocateMemory(property);

    Device::getInstance()->getDevice().bindBufferMemory(buffer_, memory_, 0);

    if (property != MemoryPropertyFlagBits::eDeviceLocal)
    {
        data_ = Device::getInstance()->getDevice().mapMemory(memory_, 0, size_);
    }
    
    return true;
}

void Buffer::release()
{
    Device::getInstance()->getDevice().unmapMemory(memory_);
    
    Device::getInstance()->getDevice().destroyBuffer(buffer_);
    Device::getInstance()->getDevice().freeMemory(memory_);
}

void Buffer::copy(const Buffer& dstBuffer)
{
    CommandBufferAllocateInfo allocate;
    allocate
        .setLevel(CommandBufferLevel::ePrimary)
        .setCommandPool(Context::getInstance()->getCommandPool())
        .setCommandBufferCount(1);

    auto cmdBuffer = Device::getInstance()->getDevice().allocateCommandBuffers(allocate)[0];

    CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuffer.begin(beginInfo);

    BufferCopy bufferCopy;
    bufferCopy
        .setSrcOffset(0)
        .setDstOffset(0)
        .setSize(dstBuffer.size());

    cmdBuffer.copyBuffer(buffer_, dstBuffer, 1, &bufferCopy);
    cmdBuffer.end();

    SubmitInfo submit;
    submit.setCommandBuffers(cmdBuffer);

    Device::getInstance()->graphicsQueue.submit(submit);
    Device::getInstance()->graphicsQueue.waitIdle();
    
    Device::getInstance()->getDevice().freeCommandBuffers(Context::getInstance()->getCommandPool(), cmdBuffer);
    release();
}

Buffer::Buffer(DeviceSize size):
size_(size)
{}

Buffer::~Buffer()
{
}

void Buffer::createBuffer(BufferUsageFlags bufferType)
{
    BufferCreateInfo createInfo;
    createInfo
        .setSize(size_)
        .setUsage(bufferType)
        .setSharingMode(SharingMode::eExclusive);
    buffer_ = Device::getInstance()->getDevice().createBuffer(createInfo);
}

void Buffer::allocateMemory(MemoryPropertyFlags property)
{
    MemoryRequirements memory = Device::getInstance()->getDevice().getBufferMemoryRequirements(buffer_);
    PhysicalDeviceMemoryProperties physicalMemory = Device::getInstance()->getPDevice().getMemoryProperties();
    uint32_t typeFilter = memory.memoryTypeBits;
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
        .setAllocationSize(memory.size)
        .setMemoryTypeIndex(typeIndex);
    
    memory_ = Device::getInstance()->getDevice().allocateMemory(allocateInfo);
}
}
