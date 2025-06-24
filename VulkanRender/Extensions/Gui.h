#pragma once
#include "vkRender.h"
#include "Release.h"
#include "Renderer.h"
#include "GLFW/glfw3.h"

namespace vkRender
{
class Renderer;
class Gui : public Release
{
public:
    static Gui* create(const Renderer& renderer);
    static void init(GLFWwindow* window);
    bool init(const Renderer& renderer);
    void release() const override;

    void Renderer(const vk::CommandBuffer& cmdBuf);

    std::function<void()> OnGui = {};
    
    Gui();
    ~Gui();
private:
};
}

