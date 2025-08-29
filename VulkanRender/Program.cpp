#include "Program.h"

#include "Context.h"
#include "Device.h"
#include "Shader.h"
#include "Buffer.h"

namespace vkRender
{
US_VKN;

Program* Program::create(const std::string& vertFile, const std::string& fragFile)
{
    Program *uniform = new (std::nothrow) Program();
    if (uniform && uniform->init(vertFile, fragFile))
    {
        return uniform;
    }
    return nullptr;
}

bool Program::init(const std::string& vertFile, const std::string& fragFile)
{
    shader_ = Shader::create(vertFile, fragFile);
    return true;
}

void Program::addImageInfo(const DescriptorImageInfo& imageInfo)
{
    imageInfos_.push_back(imageInfo);
}

void Program::setUniform(int currentFrame, const void* data)
{
    uniformBuffers_[currentFrame]->data(data);
}

void Program::setDescriptor(const std::vector<DescriptorType>& bindings)
{
    WriteDescriptorSet set;
    std::vector<DescriptorPoolSize> poolSizes;
    
    for (int i = 0; i < bindings.size(); ++i)
    {
        auto binding = bindings[i];
        poolSizes.emplace_back(binding, MAX_FRAME_IN_FLIGHT);
        
        if (binding == DescriptorType::eUniformBuffer || binding == DescriptorType::eUniformBufferDynamic)
        {
            for (int uniformBufCount = 0; uniformBufCount < MAX_FRAME_IN_FLIGHT; ++uniformBufCount)
            {
                uniformBuffers_[uniformBufCount] = Buffer::create(BufferUsageFlagBits::eUniformBuffer, sizeof(UniformObj));
            }
        }
        if (binding == DescriptorType::eCombinedImageSampler)
        {
            set.setImageInfo(imageInfos_[i % imageInfos_.size()]);
        }
        
        set.setDstBinding(i).setDescriptorType(binding);
        writes_.push_back(set);
    }
    createDescriptorPool(poolSizes);
    createDescriptorSets();
}

void Program::setBinding(const std::vector<VertexInputBindingDescription>& binding)
{
    vertexInput_.binding = binding;
}

void Program::setAttribute(const std::vector<VertexInputAttributeDescription>& attribute)
{
    vertexInput_.attribute = attribute;
}

void Program::use(const CommandBuffer& cmdBuf, int currentFrame) const
{
    cmdBuf.bindPipeline(PipelineBindPoint::eGraphics, graphicsPipeline_);
    cmdBuf.bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout_, 0, 1, &descriptorSets_[currentFrame], 0, nullptr);
}

void Program::compile(const RenderPass &renderPass)
{
    createPipelineLayout();
    createPipeline(renderPass, vertexInput_);
}

void Program::release() const
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


void Program::createDescriptorPool(const std::vector<DescriptorPoolSize>& poolSizes)
{
    DescriptorPoolCreateInfo createInfo;
    createInfo
        .setPoolSizes(poolSizes)
        .setMaxSets(MAX_FRAME_IN_FLIGHT);

    descriptorPool_ = Device::Instance()->getDevice().createDescriptorPool(createInfo);
}

void Program::createDescriptorSets()
{
    std::vector layouts(MAX_FRAME_IN_FLIGHT, shader_->getSetLayout());
    
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
                auto info = uniformBuffers_[i]->newDescriptor();
                write.setBufferInfo(info);
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

void Program::createPipeline(const RenderPass &renderPass, const VertexInputInfo& vertexInput)
{
    GraphicsPipelineCreateInfo createInfo;
    
    std::array shaderStages = {shader_->vert, shader_->frag};
    createInfo.setStages(shaderStages);
    
    constexpr std::array dynamicStates = {DynamicState::eViewport , DynamicState::eScissor};
    PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.setDynamicStates(dynamicStates);
    createInfo.setPDynamicState(&dynamicStateCreateInfo);

    
    // auto attribute = Vertex::getAttribute();
    // auto bindings = Vertex::getBinding();
    // 顶点输入
    PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo
        .setVertexAttributeDescriptions(vertexInput.attribute)
        .setVertexBindingDescriptions(vertexInput.binding);
    createInfo.setPVertexInputState(&vertexInputInfo);

    // 输入汇编
    PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
    createInfo.setPInputAssemblyState(&inputAssemblyInfo);

    // 视口和剪裁矩形
    Extent2D frameSize = Context::getInstance()->getFrameSize();
    Viewport viewport {0, 0, static_cast<float>(frameSize.width), static_cast<float>(frameSize.height), 1.f};
    Rect2D scissor {0, frameSize};
    
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
