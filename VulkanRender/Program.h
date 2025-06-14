#pragma once

#include "Buffer.h"
#include "Shader.h"
#include "vkRender.h"

namespace vkRender
{


class Program
{
public:
    static Program *create(const vk::RenderPass &renderPass, float w, float h);

    bool init(const vk::RenderPass &renderPass, float w, float h);
    void addImageInfo(vk::DescriptorImageInfo imageInfo);
    
    // void getUniform();
    void setUniform(int currentFrame, const void* data);
    void use(const vk::CommandBuffer &cmdBuf, int currentFrame);
    
    void release();
private:
    Shader *shader_ = nullptr;
    vk::Pipeline graphicsPipeline_;
    vk::PipelineLayout pipelineLayout_;
    
    vk::DescriptorSetLayout pipelineSetLayout_;
    vk::DescriptorPool descriptorPool_;
    std::vector<Buffer*> uniformBuffers_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    std::vector<vk::WriteDescriptorSet> writes;
    
    void createDescriptorPool();
    void createDescriptorSets(vk::DescriptorImageInfo imageInfo);
    // void createDescriptorSetLayout();
    void createPipelineLayout();
    void createPipeline(const vk::RenderPass &renderPass, float w, float h);
};
}
