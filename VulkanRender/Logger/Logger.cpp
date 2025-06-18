#include "Logger.h"

#include "fmt/args.h"
#include "fmt/color.h"

namespace vkRender
{
void Logger::log(const char* massage)
{
    auto format = fg(fmt::color::white_smoke);
    
    fmt::print(format, "[{}] {}!\n", "Info", massage);
}
}
