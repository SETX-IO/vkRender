#pragma once

#include "Context.h"
#include "Program.h"
#include "vkRender.h"
#include "Texture.h"
#include "Swapchain.h"
#include "Vertex.h"

namespace vkRender
{
// TODO: 将 Uniform 相关的功能抽取至 Uniform 类中
class Renderer
{
public:
    static Renderer* create();

    bool init();
    void release() const;
    
    void addVertexData(const std::vector<Vertex>& vertices);
    void addIndexData(const std::vector<uint32_t>& indices);
    void addIndexData(const std::vector<uint16_t>& indices);

    void setProgram(Program* program);
    Swapchain* getSwapchain() const {return swapchain_;}

    void draw();
private:
    Texture *texture_ = nullptr;
    Swapchain *swapchain_ = nullptr;
    std::unique_ptr<Program> program_ = nullptr;

    std::unique_ptr<Buffer> vertexBuffer_ = nullptr;
    std::unique_ptr<Buffer> indexBuffer_ = nullptr;
    
    std::vector<vk::CommandBuffer> cmdBuffers_;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;

    int currentFrame = 0;

    glm::vec2 frameSize = glm::vec2(640, 480);
    
    void updateUniform() const;
};
}
