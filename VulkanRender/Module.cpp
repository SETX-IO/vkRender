#define TINYOBJLOADER_IMPLEMENTATION

#include "Module.h"
#include "Buffer.h"
#include "Texture.h"
#include <tiny_obj_loader.h>

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

Module* Module::createFormData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& textureName)
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
    std::vector<Vertex> vertexes;
    std::vector<uint32_t> indices;

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &err, moduleName.c_str());
    for (auto shape : shapes)
    {
        for (auto index : shape.mesh.indices)
        {
            Vertex vertex{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],

                attrib.texcoords[2 * index.vertex_index + 0],
                1.f - attrib.texcoords[2 * index.vertex_index + 1],
            };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertexes.size());
                vertexes.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
    
    init(vertexes, indices, textureName);
    return true;
}

bool Module::init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
    const std::string& textureName)
{
    indexCount = indices.size();
    
    texture_ = Texture::createFormFile(textureName);
    
    auto stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(vertices[0]) * vertices.size());
    stagingBuffer->data(vertices.data());
    vertexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eVertexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*vertexBuffer_);
    
    stagingBuffer = Buffer::create(BufferUsageFlagBits::eTransferSrc, sizeof(indices[0]) * indices.size());
    stagingBuffer->data(indices.data());
    indexBuffer_ = Buffer::createDeviceLocal(BufferUsageFlagBits::eIndexBuffer | BufferUsageFlagBits::eTransferDst, stagingBuffer->size());
    stagingBuffer->copy(*indexBuffer_);
    
    return true;
}

void Module::Update()
{
}

void Module::Renderer(const CommandBuffer& cmdBuf, uint32_t instanceCount) const
{
    DeviceSize offset = 0;
    cmdBuf.bindVertexBuffers(0, vertexBuffer_->getBuffer(), offset);
    cmdBuf.bindIndexBuffer(indexBuffer_->getBuffer(), 0, IndexType::eUint32);
    cmdBuf.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void Module::destroy() const
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

Module::~Module()
{
}
}
