#pragma once

#include "vkRender.h"

namespace vkRender
{
struct Vertex
{
    float x, y, w, h;
    
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
        std::array<vk::VertexInputAttributeDescription, 2> attributes;
        attributes[0]
            .setFormat(vk::Format::eR32G32Sfloat)
            .setLocation(0)
            .setOffset(0);

        attributes[1]
            .setFormat(vk::Format::eR32G32Sfloat)
            .setLocation(1)
            .setOffset(sizeof(float) * 2);
    
        return attributes;
    }
};
}