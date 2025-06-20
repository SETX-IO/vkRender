#pragma once

#include "Buffer.h"
#include "Shader.h"
#include "vkRender.h"

namespace vkRender
{
class Program
{
public:
    static Program *create(const std::string& vertFile, const std::string& fragFile);

    bool init(const std::string& vertFile, const std::string& fragFile);
    void addImageInfo(const vk::DescriptorImageInfo& imageInfo);
    
    // void getUniform();
    void setUniform(int currentFrame, const void* data);
    void setBinding(const std::vector<vk::DescriptorType> &bindings);
    void use(const vk::CommandBuffer &cmdBuf, int currentFrame);
    void compile(const vk::RenderPass &renderPass);
    
    void release();
private:
    Shader *shader_ = nullptr;
    vk::Pipeline graphicsPipeline_;
    vk::PipelineLayout pipelineLayout_;
    
    // vk::DescriptorSetLayout pipelineSetLayout_;
    vk::DescriptorPool descriptorPool_;
    std::vector<Buffer*> uniformBuffers_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    std::vector<vk::DescriptorImageInfo> imageInfos_;
    std::vector<vk::WriteDescriptorSet> writes_;
    
    void createDescriptorPool(const std::vector<vk::DescriptorPoolSize> &poolSizes);
    void createDescriptorSets();
    void createPipelineLayout();
    void createPipeline(const vk::RenderPass &renderPass);
};
}
