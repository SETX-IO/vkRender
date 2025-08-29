#include "Renderer.h"

#include "Camera.h"
#include "Buffer.h"
#include "Device.h"
#include "Context.h"
#include "Module.h"
#include "Program.h"
#include "Swapchain.h"
#include "CommandManager.h"

namespace vkRender
{
US_VKN;
Renderer* Renderer::create()
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
    const auto device = Device::Instance();
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
    {
        imageAvailableSemaphores[i] = device->newSemaphore();
        renderFinishedSemaphores[i] = device->newSemaphore();
        inFlightFences[i] = device->newFence();
    }
    swapchain_ = Swapchain::create();
    cmdBuffers_ = CommandManager::Instance()->newCmdBuffers(MAX_FRAME_IN_FLIGHT);
    
    return true;
}

void Renderer::release() const
{
    Device::Instance()->presentQueue.waitIdle();
    CommandManager::Instance()->release();
    swapchain_->release();
    program_->release();
    instanceBuffer_->release();
    for (auto module : modules_)
    {
        module->destroy();
        module = nullptr;
    }
}

Renderer& Renderer::addVertexData(const std::vector<glm::vec3>& vertices)
{
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertices[0]) * vertices.size());
    stagingBuffer->data(vertices.data());
    
    instanceBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*instanceBuffer_);

    return *this;
}

Renderer& Renderer::addModule(Module* module)
{
    modules_.push_back(module);
    module = nullptr;

    return *this;
}

Renderer& Renderer::setProgram(Program* program)
{
    program_ = program;
    program = nullptr;

    return *this;
}

void Renderer::update()
{
    for (auto module : modules_)
    {
        module->Update();
    }
    updateUniform();
}

void Renderer::draw()
{
    const auto device = Device::Instance()->getDevice();

    device.waitForFences(inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
    
    const auto res =  device.acquireNextImageKHR(swapchain_->get(), std::numeric_limits<uint32_t>::max(), imageAvailableSemaphores[currentFrame], nullptr);

    if (res.result == Result::eErrorOutOfDateKHR)
    {
        swapchain_->reCreate();
        return;
    }
    else if (res.result != Result::eSuccess && res.result != Result::eSuboptimalKHR)
    {
        std::cout << "[Vulkan] GetImageIndex Error" << "\n";
        return;
    }
    
    uint32_t imageIndex = res.value;
    cmdBuffers_[currentFrame].reset();
    device.resetFences(inFlightFences[currentFrame]);
    
    constexpr CommandBufferBeginInfo beginInfo;

    std::array<ClearValue, 2> clearValues;
    clearValues[0].setColor({0.1f, 0.1f, 0.1f, 1.0f});
    clearValues[1].setDepthStencil({1.f, 0});

    RenderPassBeginInfo passBeginInfo = swapchain_->newRenderPassBeginInfo(currentFrame);
    auto frameSize = Context::getInstance()->getFrameSize();
    const Viewport viewport{0, 0, static_cast<float>(frameSize.width), static_cast<float>(frameSize.height), 0.f, 1.f};
    const Rect2D scissor{0, frameSize};
    
    passBeginInfo
        .setClearValues(clearValues)
        .setRenderArea(scissor);
    
    cmdBuffers_[currentFrame].begin(beginInfo);
    cmdBuffers_[currentFrame].beginRenderPass(&passBeginInfo, {});
    
    cmdBuffers_[currentFrame].setViewport(0, 1, &viewport);
    cmdBuffers_[currentFrame].setScissor(0, 1, &scissor);

    for (auto module : modules_)
    {
        DeviceSize offset = 0;
        program_->use(cmdBuffers_[currentFrame], currentFrame);
        cmdBuffers_[currentFrame].bindVertexBuffers(1, instanceBuffer_->getBuffer(), offset);
        module->Renderer(cmdBuffers_[currentFrame], 3);
    }
    
    cmdBuffers_[currentFrame].endRenderPass();
    cmdBuffers_[currentFrame].end();
    
    SubmitInfo submitInfo;
    constexpr std::array<PipelineStageFlags, 1> waitStages = {PipelineStageFlagBits::eColorAttachmentOutput};
    
    submitInfo
        .setWaitSemaphores(imageAvailableSemaphores[currentFrame])
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(cmdBuffers_[currentFrame])
        .setSignalSemaphores(renderFinishedSemaphores[currentFrame]);

    Device::Instance()->graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

    PresentInfoKHR presentInfo;
    presentInfo
        .setWaitSemaphores(renderFinishedSemaphores[currentFrame])
        .setSwapchains(swapchain_->get())
        .setPImageIndices(&imageIndex);

    const auto result = Device::Instance()->presentQueue.presentKHR(&presentInfo);
    if (result == Result::eErrorOutOfDateKHR || result == Result::eSuboptimalKHR)
    {
        swapchain_->reCreate();
    }
    else if (result != Result::eSuccess)
    {
        std::cout << "[Vulkan] Present Error" << "\n";
        return;
    }
    
    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

Renderer::Renderer():
swapchain_(nullptr),
program_(nullptr),
instanceBuffer_(nullptr)
{
}

void Renderer::updateUniform() const
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    auto camera = Camera::Instance();
    UniformObj obj;
    obj.module = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    obj.view = camera->getView();
    obj.proj = camera->getProj();

    program_->setUniform(currentFrame, &obj);
}
}
