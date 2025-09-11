# AsTestTool 跨平台设备测试工具设计方案

## 1. 项目概述

### 1.1 项目目标
开发一个跨平台的设备测试工具AsTestTool，用于测试身份证阅读器、摄像头/高拍仪和外接手写屏三种设备。

### 1.2 技术栈
- **编程语言**: C++17
- **构建系统**: CMake 3.16+
- **GUI框架**: ImGui 1.92.2
- **图形后端**: OpenGL 3.3+
- **平台支持**: Windows (DirectShow), Linux (V4L2)
- **依赖管理**: CMake FetchContent

## 2. 系统架构设计

### 2.1 整体架构
采用分层架构模式，分为以下层次：

```
┌─────────────────────────────────────┐
│            GUI Layer                │  <- ImGui界面层
├─────────────────────────────────────┤
│         Application Layer           │  <- 应用逻辑层
├─────────────────────────────────────┤
│          Device Layer               │  <- 设备抽象层
├─────────────────────────────────────┤
│         Platform Layer              │  <- 平台适配层
└─────────────────────────────────────┘
```

### 2.2 核心模块设计

#### 2.2.1 设备抽象层 (Device Abstraction Layer)
```cpp
// 基础设备接口
class IDevice {
public:
    virtual ~IDevice() = default;
    virtual bool Initialize() = 0;
    virtual bool IsConnected() = 0;
    virtual void Shutdown() = 0;
    virtual std::string GetDeviceInfo() = 0;
};

// 身份证阅读器接口
class IIDCardReader : public IDevice {
public:
    virtual bool ReadCard(IDCardInfo& info) = 0;
    virtual std::vector<std::string> GetSupportedReaders() = 0;
};

// 摄像头接口
class ICamera : public IDevice {
public:
    virtual bool StartPreview() = 0;
    virtual bool StopPreview() = 0;
    virtual bool CaptureImage(std::vector<uint8_t>& imageData) = 0;
    virtual std::vector<CameraInfo> GetAvailableCameras() = 0;
    virtual bool SetResolution(int width, int height) = 0;
};

// 手写屏接口
class ISignaturePad : public IDevice {
public:
    virtual bool StartCapture() = 0;
    virtual bool StopCapture() = 0;
    virtual std::vector<SignaturePoint> GetSignatureData() = 0;
    virtual bool ClearSignature() = 0;
};
```

#### 2.2.2 平台适配层 (Platform Adapter Layer)
```cpp
// Windows平台实现
#ifdef _WIN32
class WindowsIDCardReader : public IIDCardReader;
class WindowsCamera : public ICamera;  // DirectShow
class WindowsSignaturePad : public ISignaturePad;
#endif

// Linux平台实现
#ifdef __linux__
class LinuxIDCardReader : public IIDCardReader;
class LinuxCamera : public ICamera;    // V4L2
class LinuxSignaturePad : public ISignaturePad;
#endif
```

#### 2.2.3 设备工厂模式
```cpp
class DeviceFactory {
public:
    static std::unique_ptr<IIDCardReader> CreateIDCardReader(const std::string& type);
    static std::unique_ptr<ICamera> CreateCamera();
    static std::unique_ptr<ISignaturePad> CreateSignaturePad(const std::string& type);
};
```

## 3. 详细模块设计

### 3.1 身份证阅读器模块

#### 3.1.1 数据模型
```cpp
struct IDCardInfo {
    std::string name;           // 姓名
    std::string gender;         // 性别
    std::string nation;         // 民族
    std::string birthDate;      // 出生日期
    std::string address;        // 住址
    std::string idNumber;       // 身份证号
    std::string issuingAuthority; // 签发机关
    std::string validPeriod;    // 有效期限
    std::vector<uint8_t> photo; // 照片数据
};
```

#### 3.1.2 插件化设计
```cpp
// DLL/SO加载器
class LibraryLoader {
public:
    bool LoadLibrary(const std::string& path);
    void* GetFunction(const std::string& name);
    void UnloadLibrary();
private:
    void* m_handle = nullptr;
};

// 身份证阅读器适配器
class IDCardReaderAdapter {
public:
    bool Initialize(const std::string& libraryPath, const std::string& config);
    bool ReadCard(IDCardInfo& info);
private:
    LibraryLoader m_loader;
    // 函数指针
    typedef int (*InitFunc)(void*);
    typedef int (*ReadCardFunc)(void*, char*, int);
    typedef int (*GetPhotoFunc)(void*, unsigned char*, int*);
    // ...
};
```

### 3.2 摄像头模块

#### 3.2.1 数据模型
```cpp
struct CameraInfo {
    std::string name;
    std::string devicePath;
    std::vector<Resolution> supportedResolutions;
    bool isAvailable;
};

struct Resolution {
    int width;
    int height;
    int fps;
};
```

#### 3.2.2 平台实现
```cpp
// Windows DirectShow实现
class WindowsCamera : public ICamera {
private:
    IGraphBuilder* m_graphBuilder = nullptr;
    ICaptureGraphBuilder2* m_captureBuilder = nullptr;
    IBaseFilter* m_cameraFilter = nullptr;
    ISampleGrabber* m_sampleGrabber = nullptr;
    IMediaControl* m_mediaControl = nullptr;
};

// Linux V4L2实现
class LinuxCamera : public ICamera {
private:
    int m_fd = -1;
    struct v4l2_capability m_cap;
    struct v4l2_format m_format;
    std::vector<uint8_t> m_buffer;
};
```

### 3.3 手写屏模块

#### 3.3.1 数据模型
```cpp
struct SignaturePoint {
    float x, y;
    float pressure;
    uint64_t timestamp;
    bool isDown;
};

struct SignatureData {
    std::vector<SignaturePoint> points;
    int width, height;
    std::string deviceInfo;
};
```

#### 3.3.2 全屏窗口管理
```cpp
class FullscreenWindow {
public:
    bool Create(int width, int height);
    void Show();
    void Hide();
    void DrawSignature(const std::vector<SignaturePoint>& points);
    bool IsVisible() const;
private:
    GLFWwindow* m_window = nullptr;
    ImGuiContext* m_context = nullptr;
};
```

## 4. GUI设计

### 4.1 界面布局
```
┌─────────────────────────────────────────────────────────┐
│                    AsTestTool v1.0                      │
├─────────────────────────────────────────────────────────┤
│  [设备选择] [身份证] [摄像头] [手写屏] [设置] [关于]      │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │身份证阅读器  │  │  摄像头     │  │  手写屏     │     │
│  │             │  │             │  │             │     │
│  │ [连接设备]   │  │ [选择设备]   │  │ [连接设备]   │     │
│  │ [读取身份证] │  │ [开始预览]   │  │ [开始测试]   │     │
│  │             │  │ [拍照]      │  │ [清除轨迹]   │     │
│  │ 设备状态:    │  │ [设置分辨率] │  │             │     │
│  │ 未连接      │  │             │  │ 设备状态:    │     │
│  └─────────────┘  └─────────────┘  │ 未连接      │     │
│                                    └─────────────┘     │
│                                                         │
│  ┌─────────────────────────────────────────────────────┐ │
│  │                信息显示区域                          │ │
│  │                                                     │ │
│  │  [身份证信息] [摄像头预览] [手写轨迹]                │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 4.2 主题设计
- **主色调**: 亮色主题 (#F5F5F5 背景, #FFFFFF 面板)
- **强调色**: 蓝色系 (#2196F3)
- **文字色**: 深灰色 (#333333)
- **边框**: 浅灰色 (#E0E0E0)

### 4.3 交互设计
1. **设备状态指示**: 绿色(已连接), 红色(未连接), 黄色(连接中)
2. **实时反馈**: 操作按钮状态变化, 进度条显示
3. **错误处理**: 友好的错误提示对话框
4. **快捷键支持**: 常用操作的键盘快捷键

## 5. 项目结构

```
AsTestTool/
├── CMakeLists.txt                 # 主CMake文件
├── README.md                      # 项目说明
├── LICENSE                        # 许可证
├── src/                          # 源代码
│   ├── main.cpp                  # 主程序入口
│   ├── core/                     # 核心框架
│   │   ├── Application.h/cpp     # 应用程序类
│   │   ├── DeviceFactory.h/cpp   # 设备工厂
│   │   └── Config.h/cpp          # 配置管理
│   ├── devices/                  # 设备模块
│   │   ├── interfaces/           # 设备接口
│   │   │   ├── IDevice.h
│   │   │   ├── IIDCardReader.h
│   │   │   ├── ICamera.h
│   │   │   └── ISignaturePad.h
│   │   ├── idcard/               # 身份证阅读器
│   │   │   ├── IDCardReaderAdapter.h/cpp
│   │   │   ├── LibraryLoader.h/cpp
│   │   │   └── platforms/        # 平台实现
│   │   ├── camera/               # 摄像头
│   │   │   ├── CameraManager.h/cpp
│   │   │   └── platforms/        # 平台实现
│   │   └── signature/            # 手写屏
│   │       ├── SignaturePadAdapter.h/cpp
│   │       ├── FullscreenWindow.h/cpp
│   │       └── platforms/        # 平台实现
│   ├── gui/                      # GUI模块
│   │   ├── MainWindow.h/cpp      # 主窗口
│   │   ├── IDCardPanel.h/cpp     # 身份证面板
│   │   ├── CameraPanel.h/cpp     # 摄像头面板
│   │   ├── SignaturePanel.h/cpp  # 手写屏面板
│   │   ├── Theme.h/cpp           # 主题管理
│   │   └── widgets/              # 自定义控件
│   └── utils/                    # 工具类
│       ├── Logger.h/cpp          # 日志系统
│       ├── FileUtils.h/cpp       # 文件工具
│       └── StringUtils.h/cpp     # 字符串工具
├── include/                      # 公共头文件
├── resources/                    # 资源文件
│   ├── fonts/                    # 字体文件
│   ├── icons/                    # 图标文件
│   └── configs/                  # 配置文件
├── third_party/                  # 第三方库
│   └── CMakeLists.txt            # 第三方库CMake
├── tests/                        # 测试代码
└── docs/                         # 文档
    ├── API.md                    # API文档
    ├── UserGuide.md              # 用户指南
    └── DeveloperGuide.md         # 开发者指南
```

## 6. 构建配置

### 6.1 CMake配置
```cmake
cmake_minimum_required(VERSION 3.16)
project(AsTestTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 平台检测
if(WIN32)
    set(PLATFORM_WINDOWS ON)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX ON)
endif()

# 依赖管理
include(FetchContent)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.2
)
FetchContent_MakeAvailable(imgui)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# 平台特定依赖
if(PLATFORM_WINDOWS)
    # DirectShow SDK
    find_package(DirectShow REQUIRED)
elseif(PLATFORM_LINUX)
    # V4L2
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(V4L2 REQUIRED libv4l2)
endif()
```

### 6.2 编译选项
```cmake
# 编译选项
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_LOGGING "Enable logging" ON)

# 优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
else()
    add_compile_options(-g -O0)
endif()
```

## 7. 错误处理和日志

### 7.1 错误处理策略
```cpp
enum class ErrorCode {
    Success = 0,
    DeviceNotFound,
    DeviceNotConnected,
    InvalidParameter,
    LibraryLoadFailed,
    FunctionNotFound,
    PlatformNotSupported
};

class AsTestToolException : public std::exception {
public:
    AsTestToolException(ErrorCode code, const std::string& message);
    ErrorCode GetErrorCode() const;
    const char* what() const noexcept override;
};
```

### 7.2 日志系统
```cpp
class Logger {
public:
    enum Level { Debug, Info, Warning, Error };
    static void Log(Level level, const std::string& message);
    static void SetLevel(Level level);
    static void SetOutputFile(const std::string& filename);
};
```

## 8. 配置管理

### 8.1 配置文件格式 (JSON)
```json
{
    "application": {
        "name": "AsTestTool",
        "version": "1.0.0",
        "theme": "light"
    },
    "devices": {
        "idcard": {
            "default_library": "",
            "libraries": {
                "reader1": {
                    "path": "libs/reader1.dll",
                    "config": "configs/reader1.json"
                }
            }
        },
        "camera": {
            "default_resolution": {
                "width": 1920,
                "height": 1080
            }
        },
        "signature": {
            "default_library": "",
            "libraries": {
                "pad1": {
                    "path": "libs/pad1.dll",
                    "config": "configs/pad1.json"
                }
            }
        }
    }
}
```

## 9. 测试策略

### 9.1 单元测试
- 使用Google Test框架
- 测试设备接口的各个方法
- 测试错误处理逻辑

### 9.2 集成测试
- 测试设备与GUI的集成
- 测试跨平台兼容性
- 测试配置文件加载

### 9.3 用户测试
- 提供测试脚本
- 记录测试结果
- 性能基准测试

## 10. 部署和分发

### 10.1 Windows部署
- 使用NSIS创建安装包
- 包含必要的运行时库
- 自动检测DirectShow SDK

### 10.2 Linux部署
- 提供AppImage格式
- 包含依赖库
- 支持主流发行版

## 11. 开发计划

### 阶段1: 基础框架 (1-2周)
- [x] 项目结构搭建
- [ ] CMake配置
- [ ] 基础类设计
- [ ] 日志系统

### 阶段2: 设备模块 (2-3周)
- [ ] 身份证阅读器模块
- [ ] 摄像头模块
- [ ] 手写屏模块

### 阶段3: GUI实现 (1-2周)
- [ ] ImGui集成
- [ ] 界面布局
- [ ] 主题设计

### 阶段4: 测试和优化 (1周)
- [ ] 单元测试
- [ ] 集成测试
- [ ] 性能优化

### 阶段5: 文档和部署 (1周)
- [ ] 用户文档
- [ ] 安装包制作
- [ ] 发布准备

## 12. 风险评估

### 12.1 技术风险
- **设备兼容性**: 不同厂家的DLL/SO接口差异
- **平台差异**: Windows和Linux的API差异
- **性能问题**: 实时预览和手写轨迹的流畅性

### 12.2 缓解措施
- 设计灵活的适配器模式
- 充分的平台测试
- 性能优化和缓存策略

## 13. 总结

本设计方案采用分层架构和插件化设计，确保代码的可维护性和扩展性。通过设备抽象层统一不同设备的接口，通过平台适配层处理跨平台差异，通过ImGui提供现代化的用户界面。

关键设计特点：
1. **模块化设计**: 各设备模块独立，便于维护和扩展
2. **跨平台兼容**: 统一的接口，平台特定的实现
3. **插件化架构**: 支持不同厂家的设备驱动
4. **现代化UI**: 基于ImGui的响应式界面
5. **完善的错误处理**: 统一的错误处理和日志系统

这个设计方案为AsTestTool提供了坚实的基础，能够满足您的所有需求，并为未来的功能扩展留出了空间。
