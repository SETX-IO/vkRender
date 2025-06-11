#pragma once

#include "Buffer.h"
#include "vkRender.h"

namespace vkRender
{
struct UniformObj
{
    glm::mat4 module;
    glm::mat4 view;
    glm::mat4 proj;
};

class Program
{
public:
    static Program *create(const vk::RenderPass &renderPass, float w, float h);

    bool init(const vk::RenderPass &renderPass, float w, float h);

    // void getUniform();
    // void setUnifotm();
    
    void release();
private:
    vk::Pipeline graphicsPipeline_;
    vk::PipelineLayout pipelineLayout_;
    
    vk::DescriptorSetLayout pipelineSetLayout_;
    vk::DescriptorPool descriptorPool_;
    std::vector<Buffer*> uniformBuffers_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    std::vector<vk::WriteDescriptorSet> writes;

    void createDescriptorPool();
    void createDescriptorSets();
    void createDescriptorSetLayout();
    void createPipelineLayout();
    void createPipeline(const vk::RenderPass &renderPass, float w, float h);
};
}
