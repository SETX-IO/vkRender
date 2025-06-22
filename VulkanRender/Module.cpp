#include "Module.h"

namespace vkRender
{
US_VKN;
Module* Module::create(const std::string& moduleName, const std::string& textureName)
{
    Module* module = new (std::nothrow) Module();
    if (module && module->init(moduleName, textureName))
    {
        return module;
    }
    return nullptr;
}

Module* Module::createFormData(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices, const std::string& textureName)
{
    Module* module = new (std::nothrow) Module();
    if (module && module->init(vertices, indices, textureName))
    {
        return module;
    }
    return nullptr;
}

bool Module::init(const std::string& moduleName, const std::string& textureName)
{
    
    return true;
}

bool Module::init(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices,
    const std::string& textureName)
{
    indexCount = indices.size();
    
    texture_ = Texture::createFormFile(textureName);
    
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertices[0]) * vertices.size());
    stagingBuffer->data(vertices.data());
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*vertexBuffer_);

    // stagingBuffer->reSize(sizeof(indices[0]) * indices.size());
    stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(indices[0]) * indices.size());
    stagingBuffer->data(indices.data());
    indexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eIndexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*indexBuffer_);
    
    return true;
}

void Module::Renderer(const CommandBuffer& cmdBuf, uint32_t instanceCount) const
{
    DeviceSize offset = 0;
    cmdBuf.bindVertexBuffers(0, vertexBuffer_->getBuffer(), offset);
    cmdBuf.bindIndexBuffer(indexBuffer_->getBuffer(), 0, IndexType::eUint16);
    cmdBuf.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void Module::destroy()
{
    vertexBuffer_->release();
    indexBuffer_->release();
    texture_->release();
}

Module::Module():
vertexBuffer_(nullptr),
indexBuffer_(nullptr),
texture_(nullptr)
{
}
}
