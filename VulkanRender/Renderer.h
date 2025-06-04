#pragma once

#include "Context.h"
#include "vkRender.h"

namespace vkRender
{
class Renderer
{
public:
    static Renderer* create(Context* context);

    bool init();

    void draw();
private:
    vk::RenderPass renderPass;
    vk::Pipeline graphicsPipeline;

    void createPipeline();
};
}
