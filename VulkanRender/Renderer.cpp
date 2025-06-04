#include "Renderer.h"

namespace vkRender
{
Renderer* Renderer::create(Context* context)
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
    return true;
}

void Renderer::draw()
{
}

void Renderer::createPipeline()
{
}
}
