#include "Renderer.h"

#include "CommandManager.h"
#include "Device.h"
#include "Shader.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui.h"
#include "Module.h"

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
    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAME_IN_FLIGHT);

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
    // ImGui_ImplVulkan_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();
    
    Device::Instance()->presentQueue.waitIdle();
    CommandManager::Instance()->release();
    swapchain_->release();
    program_->release();
    for (auto module : modules_)
    {
        module->destroy();
        module = nullptr;
    }
}

void Renderer::addVertexData(const std::vector<Vertex>& vertices)
{
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertices[0]) * vertices.size());
    stagingBuffer->data(vertices.data());
    
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*vertexBuffer_);
}

void Renderer::addModule(Module* module)
{
    modules_.push_back(module);
    module = nullptr;
}

void Renderer::setProgram(Program* program)
{
    program_ = program;
    program = nullptr;
}

void Renderer::draw()
{
    const auto device = Device::Instance()->getDevice();

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

    constexpr DeviceSize offset = 0;

    for (auto module : modules_)
    {
        program_->use(cmdBuffers_[currentFrame], currentFrame);
        module->Renderer(cmdBuffers_[currentFrame], 1);
    }

    // // Start the Dear ImGui frame
    // ImGui_ImplVulkan_NewFrame();
    // ImGui::NewFrame();
    //
    // // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    // // if (show_demo_window)
    // ImGui::ShowDemoWindow();
    //
    // bool isOpen = true;
    // ImGui::Begin("A", &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    //
    // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // ImGui::Text("Gpu: %s", Device::Instance()->properties.deviceName.data());
    //
    // ImGui::End();
    //
    // ImGui::Render();
    // ImDrawData* draw_data = ImGui::GetDrawData();
    // const bool is_minimized = draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f;
    // if (!is_minimized) {
    //     ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBuffers_[currentFrame]);
    // }
    
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

    Device::Instance()->graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

    PresentInfoKHR presentInfo;
    presentInfo
        .setWaitSemaphores(renderFinishedSemaphores[currentFrame])
        .setSwapchains(swapchain_->get())
        .setPImageIndices(&imageIndex);

    if (const auto result = Device::Instance()->presentQueue.presentKHR(&presentInfo); result == Result::eErrorOutOfDateKHR)
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
