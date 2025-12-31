#include "QueueFamilyIndices.h"
#include "vulkan/vulkan.hpp"

namespace vkRender
{
QueueFamilyIndices::operator bool() const
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

QueueFamilyIndices QueueFamilyIndices::find(const vk::PhysicalDevice& gpu, const vk::SurfaceKHR& surface)
{
    QueueFamilyIndices familyIndices;
    std::vector familyProperties = gpu.getQueueFamilyProperties();

    for (int i = 0; i < familyProperties.size(); ++i)
    {
        const auto& familyProperty = familyProperties[i];
        if (familyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            familyIndices.graphicsFamily = i;
        }

        if (gpu.getSurfaceSupportKHR(i, surface))
        {
            familyIndices.presentFamily = i;
        }

        if (familyIndices)
        {
            break;
        }
    }

    return familyIndices;
}

}
