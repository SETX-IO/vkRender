#include "Renderer.h"

#include "CommandManager.h"
#include "Device.h"
#include "Shader.h"
#include "Module.h"

#include "imgui.h"

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

    imgui = Gui::create(*this);
    imgui->OnGui = []
    {
        ImGui::ShowDemoWindow();
        
        bool isOpen = true;
        ImGui::Begin("A", &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); 
        ImGui::Text("Gpu: %s", Device::Instance()->properties.deviceName.data());
    
        ImGui::End();    
    };
    
    return true;
}

void Renderer::release() const
{
    Device::Instance()->presentQueue.waitIdle();

    imgui->release();
    CommandManager::Instance()->release();
    swapchain_->release();
    program_->release();
    for (auto module : modules_)
    {
        module->destroy();
        module = nullptr;
    }
}

Renderer& Renderer::addVertexData(const std::vector<Vertex>& vertices)
{
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertices[0]) * vertices.size());
    stagingBuffer->data(vertices.data());
    
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*vertexBuffer_);

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
        std::cout << "?" << "\t";
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
    const Viewport viewport{0, 0, frameSize.x, frameSize.y, 0.f, 1.f};
    const Rect2D scissor{{0, 0}, {static_cast<uint32_t>(frameSize.x), static_cast<uint32_t>(frameSize.y)}};
    
    passBeginInfo
        .setClearValues(clearValues)
        .setRenderArea(scissor);
    
    cmdBuffers_[currentFrame].begin(beginInfo);
    cmdBuffers_[currentFrame].beginRenderPass(&passBeginInfo, {});
    
    cmdBuffers_[currentFrame].setViewport(0, 1, &viewport);
    cmdBuffers_[currentFrame].setScissor(0, 1, &scissor);

    for (auto module : modules_)
    {
        program_->use(cmdBuffers_[currentFrame], currentFrame);
        module->Renderer(cmdBuffers_[currentFrame], 1);
    }

    imgui->Renderer(cmdBuffers_[currentFrame]);
    
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
        std::cout << "???" << "\t";
        return;
    }
    
    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

Renderer::Renderer():
swapchain_(nullptr),
program_(nullptr),
vertexBuffer_(nullptr),
imgui(nullptr)
{}

void Renderer::updateUniform() const
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
