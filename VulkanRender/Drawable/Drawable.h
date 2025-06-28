#pragma once

struct Drawable
{
    virtual void Renderer(const vk::CommandBuffer&, uint32_t) const = 0;
    virtual void Update() = 0;

    virtual ~Drawable() = default;
};
