#include "Renderer.h"

#include "CommandManager.h"
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
    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAME_IN_FLIGHT);

    const auto device = Device::getInstance();
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        imageAvailableSemaphores[i] = device->newSemaphore();
        renderFinishedSemaphores[i] = device->newSemaphore();
        inFlightFences[i] = device->newFence();
    }
    swapchain_ = Swapchain::create();
    cmdBuffers_ = CommandManager::Instance()->newCmdBuffers(MAX_FRAME_IN_FLIGHT);
    texture_ = Texture::createFormFile("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/image.jpg");
    
    createBuffer();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets();
    createPipeline();
    return true;
}

void Renderer::release() const
{
    Device::getInstance()->presentQueue.waitIdle();
    CommandManager::Instance()->release();
    swapchain_->release();
    Device::getInstance()->getDevice().destroyPipeline(graphicsPipeline);
    Device::getInstance()->getDevice().destroyPipelineLayout(pipelineLayout);
    Device::getInstance()->getDevice().destroyDescriptorSetLayout(pipelineSetLayout);
    Device::getInstance()->getDevice().destroyDescriptorPool(descriptorPool_);
    texture_->release();
    vertexBuffer_->release();
    indexBuffer_->release();
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        uniformBuffer_[i]->release();
    }
    
}

void Renderer::draw()
{
    const auto device = Device::getInstance()->getDevice();

    const auto res1 = device.waitForFences(inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
    if (res1 != Result::eSuccess)
    {
        std::cout << "????" << "\t";
        return;
    }
    device.resetFences(inFlightFences[currentFrame]);

    const auto res =  device.acquireNextImageKHR(swapchain_->get(), std::numeric_limits<uint32_t>::max(), imageAvailableSemaphores[currentFrame], nullptr);
    if (res.result == Result::eErrorOutOfDateKHR)
    {
        swapchain_->reCreate();
        return;
    }
    if (res.result != Result::eSuccess)
    {
        std::cout << "?" << "\t";
        return;
    }
    
    uint32_t imageIndex = res.value;
    cmdBuffers_[currentFrame].reset();
    
    CommandBufferBeginInfo beginInfo;

    cmdBuffers_[currentFrame].begin(beginInfo);
    std::array<ClearValue, 2> clearValues;
    clearValues[0].setColor({0.1f, 0.1f, 0.1f, 1.0f});
    clearValues[1].setDepthStencil({1.f, 0});
    
    RenderPassBeginInfo PassBeginInfo;
    PassBeginInfo.setRenderPass(swapchain_->getRenderPass());
    PassBeginInfo.setFramebuffer(swapchain_->getFrameBuffers()[currentFrame]);
    PassBeginInfo.setRenderArea({{0, 0}, {static_cast<uint32_t>(frameSize.x),static_cast<uint32_t>(frameSize.y)}});
    PassBeginInfo.setClearValues(clearValues);

    cmdBuffers_[currentFrame].beginRenderPass(&PassBeginInfo, {});
    cmdBuffers_[currentFrame].bindPipeline(PipelineBindPoint::eGraphics, graphicsPipeline);

    const Viewport viewport{0, 0, frameSize.x, frameSize.y, 0.f, 1.f};
    const Rect2D scissor{{0, 0}, {static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)}};
    
    cmdBuffers_[currentFrame].setViewport(0, 1, &viewport);
    cmdBuffers_[currentFrame].setScissor(0, 1, &scissor);

    constexpr DeviceSize offset = 0;

    cmdBuffers_[currentFrame].bindVertexBuffers(0, vertexBuffer_->getBuffer(), offset);
    cmdBuffers_[currentFrame].bindIndexBuffer(indexBuffer_->getBuffer(), 0, IndexType::eUint16);
    cmdBuffers_[currentFrame].bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets_[currentFrame], 0 , nullptr);
    cmdBuffers_[currentFrame].drawIndexed(12, 1, 0, 0, 0);
    
    cmdBuffers_[currentFrame].endRenderPass();
    cmdBuffers_[currentFrame].end();
    
    updateUniform();

    SubmitInfo submitInfo;
    constexpr std::array<PipelineStageFlags, 1> waitStages = {PipelineStageFlagBits::eColorAttachmentOutput};
    
    submitInfo
        .setWaitSemaphores(imageAvailableSemaphores[currentFrame])
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(cmdBuffers_[currentFrame])
        .setSignalSemaphores(renderFinishedSemaphores[currentFrame]);

    Device::getInstance()->graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

    PresentInfoKHR presentInfo;
    presentInfo
        .setWaitSemaphores(renderFinishedSemaphores[currentFrame])
        .setSwapchains(swapchain_->get())
        .setPImageIndices(&imageIndex);

    if (const auto result = Device::getInstance()->presentQueue.presentKHR(&presentInfo); result == Result::eErrorOutOfDateKHR)
    {
        swapchain_->reCreate();
    }
    else if (result != Result::eSuccess)
    {
        std::cout << "???" << "\t";
        return;
    }
    
    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

void Renderer::createDescriptorPool()
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

void Renderer::createDescriptorSets()
{
    std::vector layouts(MAX_FRAME_IN_FLIGHT, pipelineSetLayout);
    descriptorSets_.resize(MAX_FRAME_IN_FLIGHT);
    
    DescriptorSetAllocateInfo allocateInfo;
    allocateInfo
        .setDescriptorPool(descriptorPool_)
        .setSetLayouts(layouts);

    descriptorSets_ = Device::getInstance()->getDevice().allocateDescriptorSets(allocateInfo);

    std::array<WriteDescriptorSet, 2> writes;

    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        auto bufferInfo = uniformBuffer_[i]->newDescriptor();
        writes[0]
            .setDstSet(descriptorSets_[i])
            .setDescriptorType(DescriptorType::eUniformBuffer)
            .setBufferInfo(bufferInfo);

        auto imageInfo = texture_->newDescriptor();
        writes[1]
            .setDstBinding(1)
            .setDstSet(descriptorSets_[i])
            .setDescriptorType(DescriptorType::eCombinedImageSampler)
            .setImageInfo(imageInfo);

        Device::getInstance()->getDevice().updateDescriptorSets(2, writes.data(), 0 ,nullptr);
    }
}

void Renderer::createDescriptorSetLayout()
{
    std::array binds = {
        DescriptorSetLayoutBinding {0, DescriptorType::eUniformBuffer, 1, ShaderStageFlagBits::eVertex},            // Uniform Buffer Binding
        DescriptorSetLayoutBinding {1, DescriptorType::eCombinedImageSampler, 1, ShaderStageFlagBits::eFragment}    // sampler Binding
    };
    
    DescriptorSetLayoutCreateInfo createInfo;
    createInfo.setBindings(binds);

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
    inputAssemblyInfo.setTopology(PrimitiveTopology::eTriangleList);
    createInfo.setPInputAssemblyState(&inputAssemblyInfo);

    // 视口和剪裁矩形
    Viewport viewport;
    viewport
        .setHeight(frameSize.y).setWidth(frameSize.x)
        .setMaxDepth(1.f);
    
    Rect2D scissor;
    scissor.setExtent({static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)});
    
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
    
    PipelineLayoutCreateInfo layoutCrateInfo;
    layoutCrateInfo.setSetLayouts(pipelineSetLayout);
    
    auto device = Device::getInstance()->getDevice();
    pipelineLayout = device.createPipelineLayout(layoutCrateInfo);
    
    createInfo
        .setLayout(pipelineLayout)
        .setRenderPass(swapchain_->getRenderPass())
        .setBasePipelineIndex(-1);

    auto result = device.createGraphicsPipeline(nullptr, createInfo);
    if (result.result != Result::eSuccess)
    {
        std::cout << "Pipeline create failed!";
    }
    graphicsPipeline = result.value;
    
    device.destroyShaderModule(vertCreateInfo.module);
    device.destroyShaderModule(fragCreateInfo.module);
}

void Renderer::createBuffer()
{
    const std::vector<Vertex> vertexes =
    {
        {-0.8f, -0.8f, 0.f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 1.f},
        {-0.8f, 0.8f, 0.f, 1.f, 1.f},

        {-0.8f, -0.8f, -0.8f, 1.f, 0.f},
        {0.8f, -0.8f, -0.8f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
    };
    
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertexes[0]) * vertexes.size());
    stagingBuffer->data(vertexes.data());
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*vertexBuffer_);

    stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(indices[0]) * indices.size());
    stagingBuffer->data(indices.data());
    
    indexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eIndexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*indexBuffer_);

    uniformBuffer_.resize(MAX_FRAME_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        uniformBuffer_[i] = Buffer::create(BufferUsageFlagBits::eUniformBuffer, sizeof(UniformObj));
    }
}

void Renderer::updateUniform()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformObj obj;
    obj.module = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    obj.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0), glm::vec3(0.f, 0.f, 1.f));
    obj.proj = glm::perspective(glm::radians(45.f), frameSize.x / frameSize.y, 0.1f, 10.f);
    obj.proj[1][1] *= -1;

    uniformBuffer_[currentFrame]->data(&obj);
}
}
