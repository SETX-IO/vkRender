#include <fmt/color.h>

#include "AttributeInfo.h"
#include "Camera.h"
#include "Context.h"
#include "Module.h"
#include "Program.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/Application.h"
#include "Platform/Window.h"

vkRender::Context *context = nullptr;
vkRender::Renderer *renderer = nullptr;

void init();
void mainLoop();
void resetCallback(GLFWwindow*, int, int);

int main(int argc, char* argv[])
{
    Application* app = new Application(true);
    Window* window_ = new Window(640, 480, "Vulkan Render");
    
    vkRender::Context::Init(app->getInstanceExtensions());
     
    context = vkRender::Context::getInstance();

    window_->setWindowSetSizeCallBack(resetCallback);
    window_->getVkSurface(context->getVkInstance(), &context->getSurface());
        
    // std::cout << argv[0] << "\n";
    init();
    
    app->setLoopCullBack(mainLoop);
    
    app->run(window_);
    
    renderer->release();
    context->release();
    
    return 0;
}

void init()
{
    context->setFrameSize(640, 480);
    vkRender::Camera::Instance()->setPos({2.f, 2.f, 2.f});

    const std::vector<vkRender::Vertex> vertexes =
    {
        // 上面
        {-0.8f, -0.8f, 0.f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 1.f},
        {-0.8f, 0.8f, 0.f, 1.f, 1.f},

        // 下面
        {-0.8f, -0.8f, -0.8f, 1.f, 0.f},
        {0.8f, -0.8f, -0.8f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},

        // 正面
        {-0.8f, 0.8f, 0.f, 1.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},

        // 背面
        {0.8f, -0.8f, 0.f, 1.f, 0.f},
        {-0.8f, -0.8f, 0.f, 0.f, 0.f},
        {-0.8f, -0.8f, -0.8f, 0.f, 1.f},
        {0.8f, -0.8f, -0.8f, 1.f, 1.f},
        
        // 左侧面
        {-0.8f, -0.8f, 0.f, 1.f, 0.f},
        {-0.8f, 0.8f, 0.f, 0.f, 0.f},
        {-0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, -0.8f, -0.8f, 1.f, 1.f},
        
        // 右侧面
        {0.8f, 0.8f, 0.f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f, 0.f},
        {0.8f, -0.8f, -0.8f, 0.f, 1.f},
        {0.8f, 0.8f, -0.8f, 1.f, 1.f},
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    const std::string resource_dir("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/");
    const std::vector<glm::vec3> instancePos = {{0, 0, -1}, {0, 0, 0}, {0, 0, 1}};
    
    auto module = vkRender::Module::createFormData(vertexes, indices, resource_dir.substr().append("image.jpg"));
    // auto module2 = vkRender::Module::createFormData(frame, {}, "");
    // auto module2 = vkRender::Module::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/viking_room.obj",
    //     "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/viking_room.png");
    
    renderer = vkRender::Renderer::create();
    
    auto program = vkRender::Program::create(resource_dir.substr().append("vert.spv"), resource_dir.substr().append("frag.spv"));
    
    program->setBinding({
        {0, sizeof(vkRender::Vertex), vk::VertexInputRate::eVertex},
        {1, sizeof(glm::vec3), vk::VertexInputRate::eInstance}
    });
    
    program->addAttribute({vk::Format::eR32G32B32Sfloat, sizeof(float) * 3})
        .addAttribute({vk::Format::eR32G32Sfloat, sizeof(float) * 2})
        .addAttribute({1, vk::Format::eR32G32B32Sfloat, sizeof(float) * 3})
        .compile(renderer->getSwapchain()->getRenderPass());
    
    program->addImageInfo(module->getTexture().newDescriptor());
    program->setDescriptor({
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eCombinedImageSampler,
    });
    
    renderer->setProgram(program)
        .addModule(module)
        .addVertexData(instancePos);
}

void resetCallback(GLFWwindow* window, int wight, int height)
{
    fmt::print(fg(fmt::color::white), "[glfw] window Size Width: {} Height: {}\n", wight, height);
    // context->setFrameSize(wight, height);
}

void mainLoop()
{
    renderer->update();
    renderer->draw();
}
