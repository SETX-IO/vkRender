#pragma once
#include "Vertex.h"
#include "vkRender.h"
#include "Drawable/Drawable.h"

namespace vkRender
{
class Buffer;
class Texture;

class Module : public Drawable
{
public:
    static Module* create(const std::string& moduleName, const std::string& textureName);
    static Module* createFormData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& textureName);

    bool init(const std::string& moduleName, const std::string& textureName);
    bool init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& textureName);

    void Update() override;
    void Renderer(const vk::CommandBuffer& cmdBuf, uint32_t instanceCount) const override;

    void destroy() const;
    
    const Texture& getTexture() const {return *texture_;}
    
    Module();
    virtual ~Module() override;
    
private:
    uint32_t indexCount = 0;
    
    Buffer* vertexBuffer_;
    Buffer* indexBuffer_;
    Texture* texture_;
};
}
