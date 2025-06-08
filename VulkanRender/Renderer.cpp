#include "Renderer.h"
#include "Device.h"
#include "Shader.h"
#include "Vertex.h"

namespace vkRender
{
US_VKN;
Renderer* Renderer::create(Context* context)
{
    Renderer *renderer = new (std::nothrow) Renderer();
    if (renderer && renderer->init())
    {
        return renderer;
    }
    return nullptr;
}

bool Renderer::init()
{
    createRenderPass();
    createDescriptorSetLayout();
    createPipeline();
    return true;
}

void Renderer::release() const
{
    Device::getInstance()->getDevice().destroyRenderPass(renderPass);
    Device::getInstance()->getDevice().destroyPipeline(graphicsPipeline);
}

void Renderer::draw()
{
}

void Renderer::createRenderPass()
{
    RenderPassCreateInfo createInfo;
    
    AttachmentDescription colorAttachment;
    colorAttachment
        .setFormat(Format::eB8G8R8A8Srgb)
        .setSamples(SampleCountFlagBits::e1)
        .setLoadOp(AttachmentLoadOp::eClear)
        .setStoreOp(AttachmentStoreOp::eStore)
        .setStencilLoadOp(AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(AttachmentStoreOp::eDontCare)
        .setInitialLayout(ImageLayout::eUndefined)
        .setFinalLayout(ImageLayout::ePresentSrcKHR);
    createInfo.setAttachments(colorAttachment);
    
    AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setLayout(ImageLayout::eColorAttachmentOptimal);

    SubpassDescription subpass;
    subpass
        .setPipelineBindPoint(PipelineBindPoint::eGraphics)
        .setColorAttachments(colorAttachmentRef);
    createInfo.setSubpasses(subpass);

    SubpassDependency dependency;
    dependency
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(AccessFlagBits::eColorAttachmentWrite);
    createInfo.setDependencies(dependency);

    renderPass = Device::getInstance()->getDevice().createRenderPass(createInfo);
}

void Renderer::createDescriptorSetLayout()
{
    DescriptorSetLayoutBinding uboBinding;
    uboBinding
        .setDescriptorCount(1)
        .setDescriptorType(DescriptorType::eUniformBuffer)
        .setStageFlags(ShaderStageFlagBits::eVertex);

    DescriptorSetLayoutBinding samplerBinding;
    samplerBinding
        .setBinding(1)
        .setDescriptorCount(1)
        .setDescriptorType(DescriptorType::eCombinedImageSampler)
        .setStageFlags(ShaderStageFlagBits::eFragment);

    std::array bindings = {uboBinding, samplerBinding};
    DescriptorSetLayoutCreateInfo createInfo;
    createInfo.setBindings(bindings);

     pipelineSetLayout = Device::getInstance()->getDevice().createDescriptorSetLayout(createInfo);
}

void Renderer::createPipeline()
{
    GraphicsPipelineCreateInfo createInfo;
    
    PipelineShaderStageCreateInfo vertCreateInfo =
        Shader::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv", ShaderStageFlagBits::eVertex);
    
    PipelineShaderStageCreateInfo fragCreateInfo
    = Shader::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv", ShaderStageFlagBits::eFragment);


    std::array shaderStages = {vertCreateInfo, fragCreateInfo};
    createInfo.setStages(shaderStages);
    
    std::vector dynamicStates = {DynamicState::eViewport, DynamicState::eScissor};
    
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
    inputAssemblyInfo
        .setTopology(PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(false);
    createInfo.setPInputAssemblyState(&inputAssemblyInfo);

    // 视口和剪裁矩形
    Viewport viewport;
    viewport
        .setHeight(frameSize.y).setWidth(frameSize.x)
        .setMaxDepth(1.f);
    
    Rect2D scissor;
    scissor
        .setExtent({static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)});
    
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
    
    PipelineLayoutCreateInfo layoutCrateInfo;
    layoutCrateInfo.setSetLayouts(pipelineSetLayout);
    
    auto device = Device::getInstance()->getDevice();
    pipelineLayout = device.createPipelineLayout(layoutCrateInfo);
    
    createInfo
        .setLayout(pipelineLayout)
        .setRenderPass(renderPass)
        .setBasePipelineIndex(-1);

    auto result = device.createGraphicsPipeline(nullptr, createInfo);
    if (result.result != Result::eSuccess)
    {
        std::cout << "Pipeline create failed!";
        // return false;
    }
    graphicsPipeline = result.value;
    
    device.destroyShaderModule(vertCreateInfo.module);
    device.destroyShaderModule(fragCreateInfo.module);

    // return true;
}
}
