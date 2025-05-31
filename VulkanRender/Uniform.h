#pragma once

#include "vkRender.h"

namespace vkRender
{
struct Uniform
{
    float r, g, b;

    static vk::DescriptorSetLayoutBinding getBinding()
    {
        vk::DescriptorSetLayoutBinding binding;
        binding
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setStageFlags(vk::ShaderStageFlagBits::eFragment)
            .setDescriptorCount(1);
        
        return binding;
    }
};
}
