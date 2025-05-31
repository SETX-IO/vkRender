#include "Context.h"

#include "Device.h"
#include "Vertex.h"

#include "glm/gtc/matrix_transform.hpp"

namespace vkRender
{
US_VKN;

Context* Context::s_instance = nullptr;

Context* Context::getInstance(const std::vector<const char*>& extensions, const CreateSurfacerFunc& func)
{
    if (!s_instance)
    {
        s_instance = new(std::nothrow) Context();
        s_instance->init(extensions, func);
    }

    return s_instance;
}

Context::~Context()
{
    const auto device = Device::getInstance()->getDevice();
    vkInstance.destroySurfaceKHR(surface);

    swapchain_->release();
    
    vkInstance.destroy();
    
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);
    device.destroyCommandPool(commandPool);
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        device.destroySemaphore(imageAvailableSemaphores[i]);
        device.destroySemaphore(renderFinishedSemaphores[i]);
        device.destroyFence(inFlightFences[i]);
        uniformBuffer[i]->release();
    }
    
    indexBuffer_->release();
    vertexBuffer_->release();

    device.destroyDescriptorPool(descriptorPool);
    device.destroyDescriptorSetLayout(pipelineSetLayout);
    
    s_instance = nullptr;
}

const std::vector<Vertex> vertexes =
    {
        {-0.8f, -0.8f},
        {0.8f, -0.8f},
        {0.8f, 0.8f},
        {-0.8f, 0.8f}
    };

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

bool Context::init(const std::vector<const char*>& extensions, const CreateSurfacerFunc& func)
{
    bool result = false;
    // create Vulkan Instance
    createVkInstance(extensions);

    if (func != nullptr)
    {
        surface = func(vkInstance);
    }
    
    createRenderPass();
    
    swapchain_ = Swapchain::create();
    pipelineSetLayout = createDescriptorSetLayout();
    result = createGraphicsPipeLine();
    
    createCommandPool();
    createCommandBuffer();
    createSycnObjcet();
    
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertexes[0]) * vertexes.size());
    memcpy(stagingBuffer->data_, vertexes.data(), stagingBuffer->size());
    
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    
    stagingBuffer->copy(*vertexBuffer_);
    // stagingBuffer->release();
    
    createIndexBuffer();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    
    return result;
}

void Context::createIndexBuffer()
{
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(indices[0]) * indices.size());
    memcpy(stagingBuffer->data_, indices.data(), stagingBuffer->size());
    
    indexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eIndexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    
    stagingBuffer->copy(*indexBuffer_);
    // stagingBuffer->release();
}

void Context::createUniformBuffer()
{
    uniformBuffer.resize(MAX_FRAME_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        uniformBuffer[i] = Buffer::create(BufferUsageFlagBits::eUniformBuffer, sizeof(UniformObj));
    }
}

void Context::createDescriptorPool()
{
    DescriptorPoolSize poolSize;
    poolSize
        .setType(DescriptorType::eUniformBuffer)
        .setDescriptorCount(MAX_FRAME_IN_FLIGHT);

    DescriptorPoolCreateInfo createInfo;
    createInfo
        .setPoolSizes(poolSize)
        .setMaxSets(MAX_FRAME_IN_FLIGHT);

    descriptorPool = Device::getInstance()->getDevice().createDescriptorPool(createInfo);    
}

void Context::createDescriptorSets()
{
    std::vector layouts(MAX_FRAME_IN_FLIGHT, pipelineSetLayout);
    descriptorSets.resize(MAX_FRAME_IN_FLIGHT);
    
    DescriptorSetAllocateInfo allocateInfo;
    allocateInfo
        .setDescriptorPool(descriptorPool)
        .setSetLayouts(layouts);

    descriptorSets = Device::getInstance()->getDevice().allocateDescriptorSets(allocateInfo);

    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        DescriptorBufferInfo bufferInfo;
        bufferInfo
            .setBuffer(uniformBuffer[i]->getBuffer())
            .setOffset(0)
            .setRange(sizeof(UniformObj));

        WriteDescriptorSet write;
        write
            .setDstSet(descriptorSets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorType(DescriptorType::eUniformBuffer)
            .setBufferInfo(bufferInfo);

        Device::getInstance()->getDevice().updateDescriptorSets(1, &write, 0 ,nullptr);
    }
}

DescriptorSetLayout Context::createDescriptorSetLayout()
{
    DescriptorSetLayoutBinding setBinding;
    setBinding
        .setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(DescriptorType::eUniformBuffer)
        .setStageFlags(ShaderStageFlagBits::eVertex);
    
    DescriptorSetLayoutCreateInfo createInfo;
    createInfo.setBindings(setBinding);

    return Device::getInstance()->getDevice().createDescriptorSetLayout(createInfo);
}

void Context::createVkInstance(const std::vector<const char*>& extensions)
{
    ApplicationInfo appInfo;
    appInfo
        .setApiVersion(VK_API_VERSION_1_4)
        .setPApplicationName("VK Render")
        .setPEngineName("Cocos2dx");

    const std::vector enableLayers = { "VK_LAYER_KHRONOS_validation" };
    InstanceCreateInfo instanceCreateInfo;

    instanceCreateInfo
        .setPEnabledLayerNames(enableLayers)
        .setPApplicationInfo(&appInfo);
    
    if (!extensions.empty())
    {
        instanceCreateInfo.setPEnabledExtensionNames(extensions);
    }

    vkInstance = createInstance(instanceCreateInfo);
}

void Context::createRenderPass()
{
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

    AttachmentReference colorAttachmentRef;
    colorAttachmentRef
        .setAttachment(0)
        .setLayout(ImageLayout::eColorAttachmentOptimal);

    SubpassDescription subpass;
    subpass
        .setPipelineBindPoint(PipelineBindPoint::eGraphics)
        .setColorAttachments(colorAttachmentRef);

    SubpassDependency dependency;
    dependency
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask({})
        .setDstStageMask(PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(AccessFlagBits::eColorAttachmentWrite);
    
    RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo
        .setAttachments(colorAttachment)
        .setSubpasses(subpass)
        .setDependencies(dependency);

    renderPass = Device::getInstance()->getDevice().createRenderPass(renderPassCreateInfo);
}

bool Context::createGraphicsPipeLine()
{
    auto vertSource = readFile("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv");
    auto fragSource = readFile("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv");

    auto vertShader = createShaderModel(vertSource);
    auto fragShader = createShaderModel(fragSource);

    GraphicsPipelineCreateInfo createInfo;
    
    PipelineShaderStageCreateInfo vertCreateInfo;
    PipelineShaderStageCreateInfo fragCreateInfo;
    
    vertCreateInfo
        .setStage(ShaderStageFlagBits::eVertex)
        .setModule(vertShader)
        .setPName("main");

    fragCreateInfo
        .setStage(ShaderStageFlagBits::eFragment)
        .setModule(fragShader)
        .setPName("main");

    std::array shaderStages = {vertCreateInfo, fragCreateInfo};
    createInfo.setStages(shaderStages);
    
    std::vector dynamicStates = {DynamicState::eViewport, DynamicState::eScissor};

    PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.setDynamicStates(dynamicStates);

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
        .setX(0).setY(0)
        .setHeight(frameSize.y).setWidth(frameSize.x)
        .setMaxDepth(1.f).setMinDepth(0.f);

    Rect2D scissor;
    scissor
        .setOffset({0, 0})
        .setExtent({static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)});

    PipelineViewportStateCreateInfo viewportState;
    viewportState
        .setViewports(viewport)
        .setScissors(scissor);

    
    createInfo.setPViewportState(&viewportState);
    
    PipelineRasterizationStateCreateInfo rasterization;
    rasterization
        .setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(PolygonMode::eFill)
        .setLineWidth(1.f)
        .setCullMode(CullModeFlagBits::eBack)
        .setFrontFace(FrontFace::eCounterClockwise)
        .setDepthBiasEnable(false)
        .setDepthBiasConstantFactor(0.f)
        .setDepthBiasClamp(0.f)
        .setDepthBiasSlopeFactor(0.f);

    createInfo.setPRasterizationState(&rasterization);

    PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState
        .setSampleShadingEnable(false)
        .setRasterizationSamples(SampleCountFlagBits::e1)
        .setMinSampleShading(1.f)
        .setAlphaToCoverageEnable(false)
        .setAlphaToOneEnable(false);

    createInfo.setPMultisampleState(&multisampleState);

    PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment
        .setColorWriteMask(ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA)
        .setBlendEnable(false)
        .setSrcColorBlendFactor(BlendFactor::eOne)
        .setDstColorBlendFactor(BlendFactor::eZero)
        .setColorBlendOp(BlendOp::eAdd)
        .setSrcAlphaBlendFactor(BlendFactor::eOne)
        .setDstAlphaBlendFactor(BlendFactor::eZero)
        .setAlphaBlendOp(BlendOp::eAdd);

    PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending
        .setLogicOpEnable(false)
        .setLogicOp(LogicOp::eCopy)
        .setAttachmentCount(1)
        .setAttachments(colorBlendAttachment)
        .setBlendConstants({0.f, 0.f, 0.f, 0.f});
    
    createInfo.setPColorBlendState(&colorBlending);
    
    PipelineLayoutCreateInfo layoutCrateInfo;
    layoutCrateInfo.setSetLayouts(pipelineSetLayout);
    
    auto device = Device::getInstance()->getDevice();
    pipelineLayout = device.createPipelineLayout(layoutCrateInfo);
    
    createInfo
        .setPDynamicState(&dynamicStateCreateInfo)
        .setLayout(pipelineLayout)
        .setRenderPass(renderPass)
        .setSubpass(0)
        .setBasePipelineIndex(-1);

    auto result = device.createGraphicsPipeline(nullptr, createInfo);
    if (result.result != Result::eSuccess)
    {
        std::cout << "Pipeline create failed!";
        return false;
    }
    graphicsPipeline = result.value;
    
    device.destroyShaderModule(vertShader);
    device.destroyShaderModule(fragShader);

    return true;
}

void Context::createCommandPool()
{
    CommandPoolCreateInfo createInfo;
    createInfo
        .setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(Device::getInstance()->indices_.graphicsFamily.value());

    commandPool = Device::getInstance()->getDevice().createCommandPool(createInfo);
}

void Context::createCommandBuffer()
{
    commandBuffers.resize(MAX_FRAME_IN_FLIGHT);
    
    CommandBufferAllocateInfo allocateInfo;
    allocateInfo
        .setCommandPool(commandPool)
        .setLevel(CommandBufferLevel::ePrimary)
        .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));
    commandBuffers = Device::getInstance()->getDevice().allocateCommandBuffers(allocateInfo);
}

void Context::createSycnObjcet()
{
    SemaphoreCreateInfo semaphoreInfo;
    FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(FenceCreateFlagBits::eSignaled);
    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAME_IN_FLIGHT);

    auto device = Device::getInstance()->getDevice();
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
        renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
        inFlightFences[i] = device.createFence(fenceInfo);
    }
}

ShaderModule Context::createShaderModel(const std::vector<char> &code)
{
    ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    ShaderModule shaderModule = Device::getInstance()->getDevice().createShaderModule(createInfo);

    return shaderModule;
}

int currentFrame = 0;

void Context::recordCommandBuffer(uint32_t imageIndex)
{
    CommandBufferBeginInfo beginInfo;

    commandBuffers[currentFrame].begin(beginInfo);
    ClearValue clearColor({{0.0f, 0.0f, 0.0f, 1.0f}});
    RenderPassBeginInfo PassBeginInfo;
    PassBeginInfo.setRenderPass(renderPass);
    PassBeginInfo.setFramebuffer(swapchain_->getFrameBuffers()[imageIndex]);
    PassBeginInfo.setRenderArea({{0, 0}, {static_cast<uint32_t>(frameSize.x),static_cast<uint32_t>(frameSize.y)}});
    PassBeginInfo.setClearValues(clearColor);

    commandBuffers[currentFrame].beginRenderPass(&PassBeginInfo, {});
    commandBuffers[currentFrame].bindPipeline(PipelineBindPoint::eGraphics, graphicsPipeline);

    Viewport viewport{0, 0, frameSize.x, frameSize.y, 0.f, 1.f};
    Rect2D scissor{{0, 0}, {static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)}};
    
    commandBuffers[currentFrame].setViewport(0, 1, &viewport);
    commandBuffers[currentFrame].setScissor(0, 1, &scissor);

    DeviceSize offset = 0;
    commandBuffers[currentFrame].bindVertexBuffers(0, vertexBuffer_->getBuffer(), offset);
    commandBuffers[currentFrame].bindIndexBuffer(indexBuffer_->getBuffer(), 0, IndexType::eUint16);
    commandBuffers[currentFrame].bindDescriptorSets(PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0 , nullptr);
    
    commandBuffers[currentFrame].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    
    commandBuffers[currentFrame].endRenderPass();
    commandBuffers[currentFrame].end();
}

void Context::updateUniform()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformObj obj;
    obj.module = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    obj.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0), glm::vec3(0.f, 0.f, 1.f));
    obj.proj = glm::perspective(glm::radians(45.f), frameSize.x / frameSize.y, 0.1f, 10.f);
    obj.proj[1][1] *= -1;

    memcpy(uniformBuffer[currentFrame]->data_, &obj, uniformBuffer[currentFrame]->size());
}

void Context::draw()
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
    commandBuffers[currentFrame].reset();

    recordCommandBuffer(imageIndex);
    updateUniform();

    SubmitInfo submitInfo;
    std::array<PipelineStageFlags, 1> waitStages = {PipelineStageFlagBits::eColorAttachmentOutput};
    
    submitInfo
        .setWaitSemaphores(imageAvailableSemaphores[currentFrame])
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(commandBuffers[currentFrame])
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

void Context::setFrameSize(const glm::vec2& size)
{
    frameSize = size;
}

}
