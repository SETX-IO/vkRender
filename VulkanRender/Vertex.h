#pragma once

#include "vkRender.h"

namespace vkRender
{
struct Vertex
{
    float x, y, z, w, h;
    
    static vk::VertexInputBindingDescription getBinding()
    {
         vk::VertexInputBindingDescription binding;
        binding
            .setBinding(0)
            .setInputRate(vk::VertexInputRate::eVertex)
            .setStride(sizeof(Vertex));
    
        return binding;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttribute()
    {
        constexpr std::array attributes = {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32Sfloat, sizeof(float) * 3},
        };
    
        return attributes;
    }
};
}