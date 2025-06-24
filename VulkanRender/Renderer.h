#pragma once

#include "vkRender.h"
#include "Context.h"
#include "Module.h"
#include "Program.h"
#include "Swapchain.h"
#include "Vertex.h"

namespace vkRender
{
class Renderer
{
public:
    static Renderer* create();

    bool init();
    void release() const;
    
    Renderer& addVertexData(const std::vector<Vertex>& vertices);
    Renderer& addModule(Module* module);

    Renderer& setProgram(Program* program);
    Swapchain* getSwapchain() const {return swapchain_;}

    void update();
    void draw();
private:
    Swapchain *swapchain_ = nullptr;
    Program* program_ = nullptr;

    Buffer* vertexBuffer_ = nullptr;
    
    std::vector<vk::CommandBuffer> cmdBuffers_;

    std::vector<vk::Semaphore> imageAvailableSemaphores {MAX_FRAME_IN_FLIGHT};
    std::vector<vk::Semaphore> renderFinishedSemaphores {MAX_FRAME_IN_FLIGHT};
    std::vector<vk::Fence> inFlightFences {MAX_FRAME_IN_FLIGHT};

    std::vector<Module *> modules_;

    int currentFrame = 0;

    glm::vec2 frameSize = glm::vec2(640, 480);
    
    void updateUniform() const;
};
}
