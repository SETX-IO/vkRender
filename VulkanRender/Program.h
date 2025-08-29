#pragma once

#include "vkRender.h"

namespace vkRender
{
struct VertexInputInfo
{
    std::vector<vk::VertexInputBindingDescription> binding;
    std::vector<vk::VertexInputAttributeDescription> attribute;
};

class Buffer;
class Shader;

class Program
{
public:
    static Program *create(const std::string& vertFile, const std::string& fragFile);

    bool init(const std::string& vertFile, const std::string& fragFile);
    void addImageInfo(const vk::DescriptorImageInfo& imageInfo);
    
    // void getUniform();
    void setUniform(int currentFrame, const void* data);
    void setDescriptor(const std::vector<vk::DescriptorType> &bindings);
    void setBinding(const std::vector<vk::VertexInputBindingDescription>& binding);
    void setAttribute(const std::vector<vk::VertexInputAttributeDescription>& attribute);
    void use(const vk::CommandBuffer &cmdBuf, int currentFrame) const;
    void compile(const vk::RenderPass &renderPass);
    
    void release() const;
private:
    Shader *shader_ = nullptr;
    VertexInputInfo vertexInput_;
    vk::Pipeline graphicsPipeline_;
    vk::PipelineLayout pipelineLayout_;
    
    // vk::DescriptorSetLayout pipelineSetLayout_;
    vk::DescriptorPool descriptorPool_;
    std::vector<Buffer*> uniformBuffers_ {MAX_FRAME_IN_FLIGHT, nullptr};
    std::vector<vk::DescriptorSet> descriptorSets_ {MAX_FRAME_IN_FLIGHT};

    std::vector<vk::DescriptorImageInfo> imageInfos_;
    std::vector<vk::WriteDescriptorSet> writes_;
    
    void createDescriptorPool(const std::vector<vk::DescriptorPoolSize> &poolSizes);
    void createDescriptorSets();
    void createPipelineLayout();
    void createPipeline(const vk::RenderPass &renderPass, const VertexInputInfo& vertexInput);
};
}
