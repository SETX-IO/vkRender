#include "CommandManager.h"
#include "Device.h"
namespace vkRender
{
US_VKN;
std::unique_ptr<CommandManager> CommandManager::s_instance = nullptr;

CommandManager* CommandManager::Instance()
{
    if (!s_instance)
    {
        s_instance = std::make_unique<CommandManager>();;
        s_instance->init();
    }
    return s_instance.get();
}

bool CommandManager::init()
{
    CommandPoolCreateInfo createInfo;
    createInfo
        .setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(Device::Instance()->indices_.graphicsFamily.value());

    pool_ = Device::Instance()->getDevice().createCommandPool(createInfo);
    
    return true;
}

void CommandManager::release() const
{
    Device::Instance()->getDevice().destroyCommandPool(pool_);
}

std::vector<CommandBuffer> CommandManager::newCmdBuffers(uint32_t count)
{
    std::vector<CommandBuffer> cmdBuffers(count);
    
    CommandBufferAllocateInfo allocateInfo;
    allocateInfo
        .setCommandPool(pool_)
        .setLevel(CommandBufferLevel::ePrimary)
        .setCommandBufferCount(count);
    cmdBuffers = Device::Instance()->getDevice().allocateCommandBuffers(allocateInfo);

    for (auto cmdBuf : cmdBuffers)
    {
        cmdBufferStack.push(cmdBuf);
    }
    
    return cmdBuffers;
}

void CommandManager::record(const std::function<void(const CommandBuffer &)>& callback)
{
    auto cmdBuf = newCmdBuffers(1)[0];

    CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf.begin(beginInfo);
    
    callback(cmdBuf);
    
    cmdBuf.end();

    SubmitInfo submit;
    submit.setCommandBuffers(cmdBuf);

    Device::Instance()->graphicsQueue.submit(submit);
    Device::Instance()->graphicsQueue.waitIdle();

    freeCmdBuffer();
}

void CommandManager::freeCmdBuffer()
{
    auto freeBuffer = cmdBufferStack.top();
    Device::Instance()->getDevice().freeCommandBuffers(pool_, freeBuffer);
    cmdBufferStack.pop();
}
}
