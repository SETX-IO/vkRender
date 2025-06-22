#pragma once

#include "vkRender.h"
#include "Context.h"
#include "Program.h"
#include "Texture.h"
#include "Swapchain.h"
#include "Vertex.h"

namespace vkRender
{
class Module;

class Renderer
{
public:
    static Renderer* create();

    bool init();
    void release() const;
    
    void addVertexData(const std::vector<Vertex>& vertices);
    void addModule(Module* module);

    void setProgram(Program* program);
    Swapchain* getSwapchain() const {return swapchain_;}

    void draw();
private:
    Swapchain *swapchain_ = nullptr;
    Program* program_ = nullptr;

    Buffer* vertexBuffer_ = nullptr;
    Buffer* indexBuffer_ = nullptr;
    
    std::vector<vk::CommandBuffer> cmdBuffers_;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;

    std::vector<Module *> modules_;

    int currentFrame = 0;

    glm::vec2 frameSize = glm::vec2(640, 480);
    
    void updateUniform() const;
};
}
