#pragma once

#include "vkRender.h"

namespace vkRender
{
struct Vertex
{
    float x, y;
    static vk::VertexInputBindingDescription getBinding()
    {
        vk::VertexInputBindingDescription binding;
        binding
            .setBinding(0)
            .setInputRate(vk::VertexInputRate::eVertex)
            .setStride(sizeof(Vertex));
    
        return binding;
    }

    static vk::VertexInputAttributeDescription getAttribute()
    {
        vk::VertexInputAttributeDescription attribute;
        attribute
            .setBinding(0)
            .setFormat(vk::Format::eR32G32Sfloat)
            .setLocation(0)
            .setOffset(0);
    
        return attribute;
    }
};
}