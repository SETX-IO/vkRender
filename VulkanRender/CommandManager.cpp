#include "CommandManager.h"
#include "Device.h"
namespace vkRender
{
US_VKN;
CommandManager* CommandManager::s_instance = nullptr;

CommandManager* CommandManager::Instance()
{
    if (!s_instance)
    {
        s_instance = new (std::nothrow) CommandManager();
        s_instance->init();
    }
    return s_instance;
}

bool CommandManager::init()
{
    CommandPoolCreateInfo createInfo;
    createInfo
        .setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(Device::getInstance()->indices_.graphicsFamily.value());

    pool_ = Device::getInstance()->getDevice().createCommandPool(createInfo);
    
    return true;
}

void CommandManager::release() const
{
    Device::getInstance()->getDevice().destroyCommandPool(pool_);
    
    s_instance = nullptr;
}

std::vector<CommandBuffer> CommandManager::newCmdBuffers(uint32_t count)
{
    std::vector<CommandBuffer> cmdBuffers(count);
    
    CommandBufferAllocateInfo allocateInfo;
    allocateInfo
        .setCommandPool(pool_)
        .setLevel(CommandBufferLevel::ePrimary)
        .setCommandBufferCount(count);
    cmdBuffers = Device::getInstance()->getDevice().allocateCommandBuffers(allocateInfo);

    for (auto cmdBuf : cmdBuffers)
    {
        cmdBufferStack.push(cmdBuf);
    }

    return cmdBuffers;
}

void CommandManager::record(std::function<void(const CommandBuffer &)> callback)
{
    auto cmdBuf = newCmdBuffers(1)[0];

    CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf.begin(beginInfo);
    
    callback(cmdBuf);
    
    cmdBuf.end();

    SubmitInfo submit;
    submit.setCommandBuffers(cmdBuf);

    Device::getInstance()->graphicsQueue.submit(submit);
    Device::getInstance()->graphicsQueue.waitIdle();

    freeCmdBuffer();
}

void CommandManager::freeCmdBuffer()
{
    auto freeBuffer = cmdBufferStack.top();
    Device::getInstance()->getDevice().freeCommandBuffers(pool_, freeBuffer);
    cmdBufferStack.pop();
}
}
