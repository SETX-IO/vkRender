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

    program_ = Program::create(swapchain_->getRenderPass(), frameSize.x, frameSize.y);
    program_->setDescriptorInfo(texture_->newDescriptor());
    
    return true;
}

void Renderer::release() const
{
    Device::getInstance()->presentQueue.waitIdle();
    CommandManager::Instance()->release();
    swapchain_->release();
    program_->release();
    texture_->release();
    vertexBuffer_->release();
    indexBuffer_->release();
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
    
    constexpr CommandBufferBeginInfo beginInfo;

    std::array<ClearValue, 2> clearValues;
    clearValues[0].setColor({0.1f, 0.1f, 0.1f, 1.0f});
    clearValues[1].setDepthStencil({1.f, 0});

    RenderPassBeginInfo PassBeginInfo = swapchain_->newRenderPassBeginInfo(currentFrame);
    PassBeginInfo.setRenderArea({{0, 0}, {static_cast<uint32_t>(frameSize.x),static_cast<uint32_t>(frameSize.y)}});
    PassBeginInfo.setClearValues(clearValues);

    const Viewport viewport{0, 0, frameSize.x, frameSize.y, 0.f, 1.f};
    const Rect2D scissor{{0, 0}, {static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)}};

    cmdBuffers_[currentFrame].begin(beginInfo);
    cmdBuffers_[currentFrame].beginRenderPass(&PassBeginInfo, {});
    
    cmdBuffers_[currentFrame].setViewport(0, 1, &viewport);
    cmdBuffers_[currentFrame].setScissor(0, 1, &scissor);

    constexpr DeviceSize offset = 0;

    cmdBuffers_[currentFrame].bindVertexBuffers(0, vertexBuffer_->getBuffer(), offset);
    cmdBuffers_[currentFrame].bindIndexBuffer(indexBuffer_->getBuffer(), 0, IndexType::eUint16);
    program_->use(cmdBuffers_[currentFrame], currentFrame);
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

    program_->setUniform(currentFrame, &obj);
}
}
