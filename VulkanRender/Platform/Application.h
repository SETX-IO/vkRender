#pragma once
#include <memory>
#include <vector>

typedef void (*LoopCullBackPtr)();

class Window;

class Application
{
public:
    Application(bool useVulkan = false);
    
    void run(Window * window);
    std::vector<const char*> getInstanceExtensions();
    void setLoopCullBack(LoopCullBackPtr cullBcak);
    
private:
    std::unique_ptr<Window> window_;
    bool useVulkan;
    
    LoopCullBackPtr loopCullBack_;
};
