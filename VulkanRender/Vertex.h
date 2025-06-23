#pragma once

#include "vkRender.h"

namespace vkRender
{
struct Vertex
{
    float x, y, z, w, h;

    bool operator==(const Vertex& vertex) const noexcept
    {
        return x == vertex.x && y == vertex.y && z == vertex.y && w == vertex.w && h == vertex.h;
    }
};


}

namespace std
{
template<>
struct hash<vkRender::Vertex>
{
    std::size_t operator()(const vkRender::Vertex& vertex) const noexcept
    {
        std::size_t h1 = std::hash<float>{}(vertex.x);
        std::size_t h2 = std::hash<float>{}(vertex.y);
        std::size_t h3 = std::hash<float>{}(vertex.z);
        std::size_t h4 = std::hash<float>{}(vertex.w);
        std::size_t h5 = std::hash<float>{}(vertex.h);
    
        return h1 + h2 + h3 + h4 + h5; // or use boost::hash_combine
    }  
};
}



