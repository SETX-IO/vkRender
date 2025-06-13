#pragma once

#include "Context.h"
#include "Program.h"
#include "vkRender.h"
#include "Texture.h"
#include "Swapchain.h"

namespace vkRender
{
// TODO: 将 Uniform 相关的功能抽取至 Uniform 类中
class Renderer
{
public:
    static Renderer* create(Context* context);

    bool init();
    void release() const;

    void draw();
private:
    Texture *texture_ = nullptr;
    
    Swapchain *swapchain_ = nullptr;

    Program *program_ = nullptr;
    
    std::vector<vk::CommandBuffer> cmdBuffers_;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;

    Buffer *vertexBuffer_ = nullptr;
    Buffer *indexBuffer_ = nullptr;

    int currentFrame = 0;

    glm::vec2 frameSize = glm::vec2(640, 480);
    
    void createBuffer();

    void updateUniform();
};
}
