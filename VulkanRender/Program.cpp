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
    shader_ = Shader::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv",
        "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv");
    
    createPipelineLayout();
    createPipeline(renderPass, w, h);
    
    return true;
}

void Program::addImageInfo(DescriptorImageInfo imageInfo)
{
    DescriptorType type = DescriptorType::eCombinedImageSampler;
    
    WriteDescriptorSet set;
    set.setDstBinding(bindingCount++)
        .setDescriptorType(type)
        .setImageInfo(imageInfo);

    poolSizes_.emplace_back(type, MAX_FRAME_IN_FLIGHT);
    writes_.push_back(set);
}

void Program::addBufferInfo(bool isDynamic)
{
    DescriptorType type = isDynamic ? DescriptorType::eUniformBufferDynamic : DescriptorType::eUniformBuffer;
    WriteDescriptorSet set;
    set.setDstBinding(bindingCount++)
        .setDescriptorType(type);

    poolSizes_.emplace_back(type, MAX_FRAME_IN_FLIGHT);
    writes_.push_back(set);
}

void Program::buildDescriptorSet()
{
    uniformBuffers_.resize(MAX_FRAME_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        uniformBuffers_[i] = Buffer::create(BufferUsageFlagBits::eUniformBuffer, sizeof(UniformObj));
    }
    createDescriptorPool();
    createDescriptorSets();
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
    Device::Instance()->getDevice().destroyPipeline(graphicsPipeline_);
    Device::Instance()->getDevice().destroyPipelineLayout(pipelineLayout_);
    shader_->release();
    // Device::getInstance()->getDevice().destroyDescriptorSetLayout(pipelineSetLayout_);
    Device::Instance()->getDevice().destroyDescriptorPool(descriptorPool_);

    for (auto uniform : uniformBuffers_)
    {
        uniform->release();
    }
}

void Program::createDescriptorPool()
{
    DescriptorPoolCreateInfo createInfo;
    createInfo
        .setPoolSizes(poolSizes_)
        .setMaxSets(MAX_FRAME_IN_FLIGHT);

    descriptorPool_ = Device::Instance()->getDevice().createDescriptorPool(createInfo);
}

void Program::createDescriptorSets()
{
    std::vector layouts(MAX_FRAME_IN_FLIGHT, shader_->getSetLayout());
    descriptorSets_.resize(MAX_FRAME_IN_FLIGHT);
    
    DescriptorSetAllocateInfo allocateInfo;
    allocateInfo
        .setDescriptorPool(descriptorPool_)
        .setSetLayouts(layouts);

    descriptorSets_ = Device::Instance()->getDevice().allocateDescriptorSets(allocateInfo);

    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        for (auto &write : writes_)
        {
            if (write.descriptorType == DescriptorType::eUniformBuffer || write.descriptorType == DescriptorType::eUniformBufferDynamic)
            {
                auto bufferInfo = uniformBuffers_[i]->newDescriptor();
                write.setBufferInfo(bufferInfo);
            }
            
            write.setDstSet(descriptorSets_[i]);
        }

        Device::Instance()->getDevice().updateDescriptorSets(writes_ ,nullptr);
    }
}

void Program::createPipelineLayout()
{
    PipelineLayoutCreateInfo createInfo;
    createInfo.setSetLayouts(shader_->getSetLayout());
    
    pipelineLayout_ = Device::Instance()->getDevice().createPipelineLayout(createInfo);
}

void Program::createPipeline(const RenderPass &renderPass, float w, float h)
{
    GraphicsPipelineCreateInfo createInfo;
    
    std::array shaderStages = {shader_->vert, shader_->frag};
    createInfo.setStages(shaderStages);
    
    constexpr std::array dynamicStates = {DynamicState::eViewport, DynamicState::eScissor};
    PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.setDynamicStates(dynamicStates);
    createInfo.setPDynamicState(&dynamicStateCreateInfo);

    
    auto attribute = Vertex::getAttribute();
    auto bindings = Vertex::getBinding();
    // 顶点输入
    PipelineVertexInputStateCreateInfo vertexInputInfo;
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

    auto result = Device::Instance()->getDevice().createGraphicsPipeline(Device::Instance()->getPipelineCache(), createInfo);
    if (result.result != Result::eSuccess)
    {
        std::cout << "Pipeline create failed!";
    }
    graphicsPipeline_ = result.value;
}
}
