#pragma once
#include "Buffer.h"
#include "Texture.h"
#include "Vertex.h"
#include "vkRender.h"

namespace vkRender
{
class Module
{
public:
    static Module* create(const std::string& moduleName, const std::string& textureName);
    static Module* createFormData(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices, const std::string& textureName);

    bool init(const std::string& moduleName, const std::string& textureName);
    bool init(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices, const std::string& textureName);

    void Renderer(const vk::CommandBuffer& cmdBuf, uint32_t instanceCount) const;

    void destroy();
    
    const Texture& getTexture() const {return *texture_;}
    
    Module();
    
private:
    uint32_t indexCount = 0;
    
    Buffer* vertexBuffer_;
    Buffer* indexBuffer_;
    Texture* texture_;
};
}