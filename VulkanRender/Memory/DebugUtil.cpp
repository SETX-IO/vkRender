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
    if (level == DebugUtilsMessageTypeFlagBitsEXT::eValidation && type == DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
    {
        fmt::print("[Validation]: {} \n", message->pMessage);
    }
    else if (type == DebugUtilsMessageSeverityFlagBitsEXT::eError)
    {
        fmt::print(fg(fmt::color::red), "[Error|Validation]: {} \n", message->pMessage);
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

void DebugUtil::Release() const
{
    Instance instance = Context::getInstance()->getVkInstance();
    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.
        getProcAddr("DestroyDebugUtilsMessengerEXT"))(instance, messenger_, nullptr);
}

void DebugUtil::createMessenger(Instance instance)
{
    DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo
        .setMessageSeverity(DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | DebugUtilsMessageSeverityFlagBitsEXT::eError | DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
        .setMessageType(DebugUtilsMessageTypeFlagBitsEXT::eValidation | DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
        .setPfnUserCallback(debugInfoPrint)
        .setPUserData(nullptr);

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(instance, createInfo, nullptr, &messenger_);
    }
}
}
