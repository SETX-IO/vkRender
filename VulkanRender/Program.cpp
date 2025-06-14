#include "Program.h"

#include "Device.h"
#include "Shader.h"
#include "Vertex.h"

namespace vkRender
{
US_VKN;

Program* Program::create(const RenderPass &renderPass, float w, float h)
{
    Program *uniform = new (std::nothrow) Program();
    if (uniform && uniform->init(renderPass, w, h))
    {
        return uniform;
    }
    return nullptr;
}

bool Program::init(const RenderPass &renderPass, float w, float h)
{
    createDescriptorSetLayout();
    createPipelineLayout();
    createPipeline(renderPass, w, h);

    uniformBuffers_.resize(MAX_FRAME_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        uniformBuffers_[i] = Buffer::create(BufferUsageFlagBits::eUniformBuffer, sizeof(UniformObj));
    }
    
    return true;
}

void Program::setDescriptorInfo(DescriptorImageInfo imageInfo)
{
    createDescriptorPool();
    createDescriptorSets(imageInfo);
}

void Program::setUniform(int currentFrame, const void* data)
{
    uniformBuffers_[currentFrame]->data(data);
}

void Program::use(const CommandBuffer& cmdBuf, int currentFrame)
{
    cmdBuf.bindPipeline(PipelineBindPoint::eGraphics, graphicsPipeline_);
    cmdBuf.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout_, 0, 1, &descriptorSets_[currentFrame], 0, nullptr);
}

void Program::release()
{
    Device::getInstance()->getDevice().destroyPipeline(graphicsPipeline_);
    Device::getInstance()->getDevice().destroyPipelineLayout(pipelineLayout_);
    Device::getInstance()->getDevice().destroyDescriptorSetLayout(pipelineSetLayout_);
    Device::getInstance()->getDevice().destroyDescriptorPool(descriptorPool_);

    for (auto uniform : uniformBuffers_)
    {
        uniform->release();
    }
}

void Program::createDescriptorPool()
{
    std::array poolSize = {
        DescriptorPoolSize{DescriptorType::eUniformBuffer, MAX_FRAME_IN_FLIGHT},
        DescriptorPoolSize{DescriptorType::eCombinedImageSampler, MAX_FRAME_IN_FLIGHT},
    };

    DescriptorPoolCreateInfo createInfo;
    createInfo
        .setPoolSizes(poolSize)
        .setMaxSets(MAX_FRAME_IN_FLIGHT);

    descriptorPool_ = Device::getInstance()->getDevice().createDescriptorPool(createInfo);
}

void Program::createDescriptorSets(DescriptorImageInfo imageInfo)
{
    std::vector layouts(MAX_FRAME_IN_FLIGHT, pipelineSetLayout_);
    descriptorSets_.resize(MAX_FRAME_IN_FLIGHT);
    
    DescriptorSetAllocateInfo allocateInfo;
    allocateInfo
        .setDescriptorPool(descriptorPool_)
        .setSetLayouts(layouts);

    descriptorSets_ = Device::getInstance()->getDevice().allocateDescriptorSets(allocateInfo);

    std::array<WriteDescriptorSet, 2> writes;

    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        auto bufferInfo = uniformBuffers_[i]->newDescriptor();
        writes[0]
            .setDstSet(descriptorSets_[i])
            .setDescriptorType(DescriptorType::eUniformBuffer)
            .setBufferInfo(bufferInfo);
        
        writes[1]
            .setDstBinding(1)
            .setDstSet(descriptorSets_[i])
            .setDescriptorType(DescriptorType::eCombinedImageSampler)
            .setImageInfo(imageInfo);

        Device::getInstance()->getDevice().updateDescriptorSets(writes ,nullptr);
    }
}

void Program::createDescriptorSetLayout()
{
    std::array binds = {
        DescriptorSetLayoutBinding {0, DescriptorType::eUniformBuffer, 1, ShaderStageFlagBits::eVertex},            // Uniform Buffer Binding
        DescriptorSetLayoutBinding {1, DescriptorType::eCombinedImageSampler, 1, ShaderStageFlagBits::eFragment}    // sampler Binding
    };
    
    DescriptorSetLayoutCreateInfo createInfo;
    createInfo.setBindings(binds);

    pipelineSetLayout_ = Device::getInstance()->getDevice().createDescriptorSetLayout(createInfo);
}

void Program::createPipelineLayout()
{
    PipelineLayoutCreateInfo createInfo;
    createInfo.setSetLayouts(pipelineSetLayout_);
    
    pipelineLayout_ = Device::getInstance()->getDevice().createPipelineLayout(createInfo);
}

void Program::createPipeline(const RenderPass &renderPass, float w, float h)
{
    GraphicsPipelineCreateInfo createInfo;
    
    PipelineShaderStageCreateInfo vertCreateInfo =
        Shader::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv", ShaderStageFlagBits::eVertex);
    
    PipelineShaderStageCreateInfo fragCreateInfo
    = Shader::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv", ShaderStageFlagBits::eFragment);
    
    std::array shaderStages = {vertCreateInfo, fragCreateInfo};
    createInfo.setStages(shaderStages);
    
    constexpr std::array dynamicStates = {DynamicState::eViewport, DynamicState::eScissor};
    PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.setDynamicStates(dynamicStates);
    createInfo.setPDynamicState(&dynamicStateCreateInfo);

    // 顶点输入
    PipelineVertexInputStateCreateInfo vertexInputInfo;
    auto attribute = Vertex::getAttribute();
    auto bindings = Vertex::getBinding();
    
    vertexInputInfo
        .setVertexAttributeDescriptions(attribute)
        .setVertexBindingDescriptions(bindings);
    createInfo.setPVertexInputState(&vertexInputInfo);

    // 输入汇编
    PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
    createInfo.setPInputAssemblyState(&inputAssemblyInfo);

    // 视口和剪裁矩形
    Viewport viewport {0, 0, w, h, 1.f};
    Rect2D scissor {0, {static_cast<uint32_t>(w), static_cast<uint32_t>(h)}};
    
    PipelineViewportStateCreateInfo viewportState;
    viewportState
        .setViewports(viewport)
        .setScissors(scissor);
    createInfo.setPViewportState(&viewportState);

    // 光栅化
    PipelineRasterizationStateCreateInfo rasterization;
    rasterization
        .setPolygonMode(PolygonMode::eFill)
        .setLineWidth(1.f)
        .setCullMode(CullModeFlagBits::eBack)
        .setFrontFace(FrontFace::eCounterClockwise);
    createInfo.setPRasterizationState(&rasterization);

    PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState
        .setRasterizationSamples(SampleCountFlagBits::e1)
        .setMinSampleShading(1.f);
    createInfo.setPMultisampleState(&multisampleState);

    PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment
        .setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA)
        .setSrcColorBlendFactor(BlendFactor::eOne)
        .setDstColorBlendFactor(BlendFactor::eZero)
        .setColorBlendOp(BlendOp::eAdd)
        .setSrcAlphaBlendFactor(BlendFactor::eOne)
        .setDstAlphaBlendFactor(BlendFactor::eZero)
        .setAlphaBlendOp(BlendOp::eAdd);

    PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending
        .setLogicOp(LogicOp::eCopy)
        .setAttachments(colorBlendAttachment);
    
    createInfo.setPColorBlendState(&colorBlending);

    PipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil
        .setDepthTestEnable(true)
        .setDepthWriteEnable(true)
        .setDepthCompareOp(CompareOp::eLess)
        .setMaxDepthBounds(1.f);
    createInfo.setPDepthStencilState(&depthStencil);
    
    createInfo
        .setLayout(pipelineLayout_)
        .setRenderPass(renderPass)
        .setBasePipelineIndex(-1);

    auto result = Device::getInstance()->getDevice().createGraphicsPipeline(nullptr, createInfo);
    if (result.result != Result::eSuccess)
    {
        std::cout << "Pipeline create failed!";
    }
    graphicsPipeline_ = result.value;
    
    Device::getInstance()->getDevice().destroyShaderModule(vertCreateInfo.module);
    Device::getInstance()->getDevice().destroyShaderModule(fragCreateInfo.module);
}
}
