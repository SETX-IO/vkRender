#pragma once
#include <optional>
#include <vector>

namespace vk
{
class PhysicalDevice;
class SurfaceKHR;
}

namespace vkRender
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    operator bool() const;
    bool equal() const {return graphicsFamily.value() == presentFamily.value();}

    static QueueFamilyIndices find(const vk::PhysicalDevice &gpu, const vk::SurfaceKHR &surface);
};

}

