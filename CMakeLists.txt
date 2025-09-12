cmake_minimum_required(VERSION 3.16)
project(AsTestTool VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 平台检测
if(WIN32)
    set(PLATFORM_WINDOWS ON)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX ON)
    add_definitions(-DPLATFORM_LINUX)
endif()

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

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

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

# OpenGL
find_package(OpenGL REQUIRED)

# 平台特定依赖
if(PLATFORM_WINDOWS)
    # Windows特定库
    set(PLATFORM_LIBS)
elseif(PLATFORM_LINUX)
    # Linux特定库
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11)
    # V4L2是内核接口，不需要额外的库
    set(PLATFORM_LIBS ${X11_LIBRARIES})
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/DeviceFactory.cpp
    src/core/Config.cpp
    src/core/ErrorCode.cpp
    src/devices/idcard/IDCardReaderAdapter.cpp
    src/devices/idcard/LibraryLoader.cpp
    src/devices/camera/CameraManager.cpp
    src/devices/signature/SignaturePadAdapter.cpp
    src/devices/signature/FullscreenWindow.cpp
    src/plugins/PluginLoader.cpp
    src/plugins/CameraManager.cpp
    src/gui/MainWindow.cpp
    src/gui/IDCardPanel.cpp
    src/gui/CameraPanel.cpp
    src/gui/SignaturePanel.cpp
    src/gui/Theme.cpp
    src/utils/LogDisplay.cpp
    src/utils/FileUtils.cpp
    src/utils/StringUtils.cpp
)

# 平台特定源文件
if(PLATFORM_WINDOWS)
    list(APPEND SOURCES
        src/devices/idcard/WindowsIDCardReader.cpp
        src/devices/camera/WindowsCamera.cpp
        src/devices/signature/WindowsSignaturePad.cpp
    )
elseif(PLATFORM_LINUX)
    list(APPEND SOURCES
        src/devices/idcard/LinuxIDCardReader.cpp
        src/devices/camera/LinuxCamera.cpp
        src/devices/signature/LinuxSignaturePad.cpp
    )
endif()

# 头文件
set(HEADERS
    include/core/Application.h
    include/core/DeviceFactory.h
    include/core/Config.h
    include/devices/interfaces/IDevice.h
    include/devices/interfaces/IIDCardReader.h
    include/devices/interfaces/ICamera.h
    include/devices/interfaces/ISignaturePad.h
    include/devices/idcard/IDCardReaderAdapter.h
    include/devices/idcard/LibraryLoader.h
    include/devices/camera/CameraManager.h
    include/devices/signature/SignaturePadAdapter.h
    include/devices/signature/FullscreenWindow.h
    include/plugins/ICameraPlugin.h
    include/plugins/PluginLoader.h
    include/plugins/CameraManager.h
    include/gui/MainWindow.h
    include/gui/IDCardPanel.h
    include/gui/CameraPanel.h
    include/gui/SignaturePanel.h
    include/gui/Theme.h
    include/utils/Logger.h
    include/utils/LogDisplay.h
    include/utils/FileUtils.h
    include/utils/StringUtils.h
)

# 创建Logger库
add_library(Logger STATIC
    src/utils/Logger.cpp
    include/utils/Logger.h
)

target_include_directories(Logger PUBLIC
    include
)

# 设置Logger库的编译定义
if(ENABLE_LOGGING)
    target_compile_definitions(Logger PRIVATE ENABLE_LOGGING)
endif()

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 构建插件
if(PLATFORM_WINDOWS)
    add_subdirectory(plugins/windows/DirectShowPlugin)
elseif(PLATFORM_LINUX)
    add_subdirectory(plugins/linux/V4L2Plugin)
endif()

# 创建ImGui库
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    imgui
    glfw
    OpenGL::GL
    Logger
    ${PLATFORM_LIBS}
)

# 平台特定链接
if(PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif(PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
endif()

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${glfw_SOURCE_DIR}/include
)

# 编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(ENABLE_LOGGING)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_LOGGING ENABLE_LOG_DISPLAY)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 安装资源文件
install(DIRECTORY resources/
    DESTINATION share/${PROJECT_NAME}/resources
)

# 测试
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 文档
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(docs
            ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
    endif()
endif()
