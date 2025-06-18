#pragma once

#include "vkRender.h"

namespace vkRender
{
template<class T>
std::string toStringHelper(T&& arg)
{
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
}

class Logger
{
public:
    void log(const char* massage);
};
}
