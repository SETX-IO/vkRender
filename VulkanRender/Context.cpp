#include "Context.h"

#include <map>

#include "CommandManager.h"
#include "Device.h"
#include "Texture.h"
#include "Vertex.h"

#include "glm/gtc/matrix_transform.hpp"
#include "Memory/Memory.h"

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

    CommandManager::Instance()->release();
    
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
    texture_->release();

    Memory::release();
    
    s_instance = nullptr;
}

const std::vector<Vertex> vertexes =
    {
        {-0.8f, -0.8f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f},
        {0.8f, 0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, 1.f, 1.f}
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
    pipelineSetLayout = createDescriptorSetLayout();
    result = createGraphicsPipeLine();
    
    swapchain_ = Swapchain::create();
    
    commandBuffers = CommandManager::Instance()->newCmdBuffers(MAX_FRAME_IN_FLIGHT);
    createSycnObjcet();
    
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertexes[0]) * vertexes.size());
    stagingBuffer->data(vertexes.data());
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*vertexBuffer_);

    stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(indices[0]) * indices.size());
    stagingBuffer->data(indices.data());
    
    indexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eIndexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*indexBuffer_);
    
    texture_ = Texture::createFormFile("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/image.jpg");
    
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();

    
    return result;
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
    std::array <DescriptorPoolSize, 2> poolSize;
    poolSize[0]
        .setType(DescriptorType::eUniformBuffer)
        .setDescriptorCount(MAX_FRAME_IN_FLIGHT);

    poolSize[1]
        .setType(DescriptorType::eCombinedImageSampler)
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

    std::array<WriteDescriptorSet, 2> writes;

    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        DescriptorBufferInfo bufferInfo;
        bufferInfo
            .setBuffer(uniformBuffer[i]->getBuffer())
            .setRange(sizeof(UniformObj));
        
        writes[0]
            .setDstSet(descriptorSets[i])
            .setDescriptorType(DescriptorType::eUniformBuffer)
            .setBufferInfo(bufferInfo);

        auto imageInfo = texture_->newDescriptor();
        writes[1]
            .setDstBinding(1)
            .setDstSet(descriptorSets[i])
            .setDescriptorType(DescriptorType::eCombinedImageSampler)
            .setImageInfo(imageInfo);

        Device::getInstance()->getDevice().updateDescriptorSets(2, writes.data(), 0 ,nullptr);
    }
}

DescriptorSetLayout Context::createDescriptorSetLayout()
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
        return false;
    }
    graphicsPipeline = result.value;
    
    device.destroyShaderModule(vertShader);
    device.destroyShaderModule(fragShader);

    return true;
}

void Context::createSycnObjcet()
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
    ClearValue clearColor({{0.1f, 0.1f, 0.1f, 1.0f}});
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
    // commandBuffers[currentFrame].bindVertexBuffers(0, buffer, offset);
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

    uniformBuffer[currentFrame]->data(&obj);
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
