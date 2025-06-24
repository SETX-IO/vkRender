#include "DebugUtil.h"

#include "Context.h"
#include "fmt/chrono.h"
#include "fmt/color.h"

namespace vkRender
{
US_VKN;
Bool32 DebugUtil::debugInfoPrint(DebugUtilsMessageSeverityFlagBitsEXT type,
    DebugUtilsMessageTypeFlagsEXT level, const DebugUtilsMessengerCallbackDataEXT* message, void* userData)
{
    fmt::text_style color; ;
    if (level == DebugUtilsMessageTypeFlagBitsEXT::eValidation)
    {
        if (type == DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
        {
            color = fg(fmt::color::white);
            fmt::print(color, "[vulkan|Validation]: {} \n", message->pMessage);
        }
        else if (type == DebugUtilsMessageSeverityFlagBitsEXT::eError)
        {
            color = fg(fmt::color::orange_red);
            fmt::print(color, "[vulkan|Validation]: {} \n", message->pMessage);
        }
    }
    else if (level == DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
    {
        fmt::print(fg(fmt::color::white), "[General]: {} \n", message->pMessage);
    }
    else if (level == DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
    {
        fmt::print("[Performance]: {} \n", message->pMessage);
    }

    return false;
}

DebugUtil* DebugUtil::create(Instance instance)
{ 
    DebugUtil *util = new (std::nothrow) DebugUtil();
    if (util && util->init(instance))
    {
        return util;
    }
    delete util;
    return nullptr;
}

bool DebugUtil::init(Instance instance)
{
    createMessenger(instance);
    return true;
}

void DebugUtil::destroy() const
{
    Instance instance = Context::getInstance()->getVkInstance();
    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.
        getProcAddr("vkDestroyDebugUtilsMessengerEXT"))(instance, messenger_, nullptr);
}

void DebugUtil::createMessenger(Instance instance)
{
    DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo
        .setMessageSeverity(DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | DebugUtilsMessageSeverityFlagBitsEXT::eError |
            DebugUtilsMessageSeverityFlagBitsEXT::eWarning | DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
        .setMessageType(DebugUtilsMessageTypeFlagBitsEXT::eValidation | DebugUtilsMessageTypeFlagBitsEXT::eGeneral | DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
        .setPfnUserCallback(debugInfoPrint)
        .setPUserData(nullptr);

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(instance, createInfo, nullptr, &messenger_);
    }
}
}
