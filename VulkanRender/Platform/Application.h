#pragma once
#include <filesystem>
#include <memory>
#include <vector>

typedef void (*LoopCullBackPtr)();

class Window;

class Application
{
public:
    Application(bool useVulkan = false);

    static Application& Current();
    
    void run(Window * window);
    void run();
    
    std::vector<const char*> getInstanceExtensions() const;
    void setLoopCullBack(LoopCullBackPtr cullBcak);
    void setResouceDir(const char* path);
    std::string getResouceDir() const;
    void setWindow(Window* window);
    Window& getWindow() const;

private:
    std::unique_ptr<Window> window_;
    std::filesystem::path resoucesDir_;
    bool useVulkan;
    
    static std::unique_ptr<Application> currentPtr_;
    
    LoopCullBackPtr loopCullBack_;
};
