# vkRender

根据[Vulkan Tutorial](https://docs.vulkan.net.cn/tutorial/latest/00_Introduction.html) 学习并构建这个渲染引擎。

一个基于Vulkan + Glfw + glm 构建的一个渲染引擎


可能内置一个自己写的DeviceMemory分配器...


补充跟多的api供以使用

# 计划
- 基础
- [x] Buffer
- [x] Shader Program
- [x] 加入无需手动释放的材质类 (Texture)
- [ ] 模型加载


- Vulkan
- [x] 实例化渲染


- DeviceMemory分配器


# 构建
## Windows
```
git close https://github.com/SETX-IO/vkRender.git

cmake -B build
```

# 库
[GLFW](https://github.com/glfw/glfw) 窗口系统


[vulkan](https://www.vulkan.org/) 渲染后端


[fmt](https://github.com/fmtlib/fmt/tree/5860688d7e5dac0f52d4ac13ff859ca04e05f0ce) 日志需要的格式化库