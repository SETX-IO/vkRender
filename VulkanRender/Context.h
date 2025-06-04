#pragma once
#include <fstream>

#include "glm/glm.hpp"

#include "vkRender.h"
#include "Swapchain.h"
#include "Buffer.h"

namespace vkRender
{
constexpr int MAX_FRAME_IN_FLIGHT = 3;
static auto empty = std::vector<const char*>();

using CreateSurfacerFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

struct UniformObj
{
    glm::mat4 module;
    glm::mat4 view;
    glm::mat4 proj;
};

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        std::cout << "not fount File: " << filename << std::endl;
        return {};
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

// TODO: 将渲染有关的提出去单独写一个 Renderer 类
class Context final
{
public:
    // get Context Instance. parameters use on initialize
    static Context* getInstance(const std::vector<const char*> &extensions = empty, const CreateSurfacerFunc& func = nullptr);

    Context() = default;
    ~Context();
    
    // get Vulkan instance
    vk::Instance & getVkInstance() {return vkInstance;}
    vk::SurfaceKHR &getSurface() {return surface;}
    vk::RenderPass &getRenderPass() {return renderPass;}
    
    bool init(const std::vector<const char*>& extensions, const CreateSurfacerFunc& func);
    
    void setFrameSize(const glm::vec2 &size);
    
    void createVkInstance(const std::vector<const char*>& extensions);
    void createRenderPass();
    bool createGraphicsPipeLine();
    void createSycnObjcet();
    vk::ShaderModule createShaderModel(const std::vector<char> &code);
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSets();
    vk::DescriptorSetLayout createDescriptorSetLayout();

    void recordCommandBuffer(uint32_t imageIndex);
    void updateUniform();
    
    void draw();
    
    static Context* s_instance;

private:
    vk::Instance vkInstance;

    Swapchain *swapchain_;
    
    vk::SurfaceKHR surface;

    vk::RenderPass renderPass;
    vk::Pipeline graphicsPipeline;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSetLayout pipelineSetLayout;
    
    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;

    Buffer *vertexBuffer_;
    Buffer *indexBuffer_;
    std::vector<Buffer*> uniformBuffer;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    glm::vec2 frameSize = glm::vec2(640, 480);
};
}
