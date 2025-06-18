#pragma once

#include "vkRender.h"

namespace vkRender
{
class CommandManager
{
public:
    static CommandManager *Instance();

    bool init();

    void release() const;
    
    std::vector<vk::CommandBuffer> newCmdBuffers(uint32_t count);
    void record(const std::function<void(const vk::CommandBuffer&)>& callback);

    void freeCmdBuffer();
private:
    vk::CommandPool pool_;
    std::stack<vk::CommandBuffer> cmdBufferStack;
    
    static CommandManager *s_instance;
};
}
