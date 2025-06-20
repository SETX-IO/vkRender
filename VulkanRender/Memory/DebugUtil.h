#pragma once
#include "vkRender.h"

namespace vkRender
{

class DebugUtil
{
public:
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugInfoPrint(vk::DebugUtilsMessageSeverityFlagBitsEXT type, vk::DebugUtilsMessageTypeFlagsEXT level,
    const vk::DebugUtilsMessengerCallbackDataEXT *message, void* userData);

    static DebugUtil *create(vk::Instance instance);

    bool init(vk::Instance instance);

    void Release() const;
private:
    VkDebugUtilsMessengerEXT messenger_ = nullptr;

    void createMessenger(vk::Instance instance);
};
}
