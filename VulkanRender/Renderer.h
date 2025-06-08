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
    void release() const;

    void draw();
private:
    vk::RenderPass renderPass;
    vk::Pipeline graphicsPipeline;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSetLayout pipelineSetLayout;

    glm::vec2 frameSize = glm::vec2(640, 480);

    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
};
}
