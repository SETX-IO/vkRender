#include "Logger.h"

#include "fmt/args.h"
#include "fmt/color.h"

namespace vkRender
{
void Logger::log(const char* massage)
{
    fmt::print(fg(fmt::color::white_smoke),
             "[{}] {}!\n", "Info", massage);
}
}
