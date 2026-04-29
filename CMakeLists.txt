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
    if(MSVC)
        add_compile_options(/O2 /DNDEBUG)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
else()
    if(MSVC)
        add_compile_options(/Od /Zi)
        # 设置MSVC编码为UTF-8
        add_compile_options(/utf-8)
    else()
        add_compile_options(-g -O0)
    endif()
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
    src/core/DeviceManager.cpp
    src/core/Config.cpp
    src/core/ErrorCode.cpp
    src/devices/idcard/IDCardReaderAdapter.cpp
    src/devices/idcard/LibraryLoader.cpp
    src/devices/signature/SignaturePadAdapter.cpp
    src/devices/signature/FullscreenWindow.cpp
    src/plugins/PluginLoader.cpp
    src/plugins/CameraManager.cpp
    src/gui/MainWindow.cpp
    src/gui/IDCardPanel.cpp
    src/gui/CameraPanel.cpp
    src/gui/SignaturePanel.cpp
    src/gui/Theme.cpp
    src/gui/LayoutManager.cpp
    src/gui/TextureRenderer.cpp
    src/gui/BasePanel.cpp
    src/utils/LogDisplay.cpp
    src/utils/FileUtils.cpp
    src/utils/StringUtils.cpp
    src/utils/FileDialog.cpp
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
    include/core/DeviceManager.h
    include/core/Config.h
    include/devices/interfaces/IDevice.h
    include/devices/interfaces/IIDCardReader.h
    include/devices/interfaces/ICamera.h
    include/devices/interfaces/ISignaturePad.h
    include/devices/interfaces/ImageData.h
    include/devices/interfaces/IPreviewController.h
    include/devices/interfaces/IImageCapture.h
    include/devices/interfaces/IImageSettings.h
    include/devices/idcard/IDCardReaderAdapter.h
    include/devices/idcard/LibraryLoader.h
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
    include/gui/LayoutManager.h
    include/gui/TextureRenderer.h
    include/gui/PanelUiHelpers.h
    include/gui/BasePanel.h
    include/utils/Logger.h
    include/utils/LogDisplay.h
    include/utils/FileUtils.h
    include/utils/StringUtils.h
    include/utils/Singleton.h
    include/utils/FileDialog.h
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
    COMPONENT applications
)

# 安装插件
install(DIRECTORY ${CMAKE_BINARY_DIR}/plugins/
    DESTINATION bin/plugins
    COMPONENT applications
    FILES_MATCHING PATTERN "*.dll" PATTERN "*.so"
)

# 安装资源文件（如果存在）
if(EXISTS ${CMAKE_SOURCE_DIR}/resources)
    install(DIRECTORY resources/
        DESTINATION share/${PROJECT_NAME}/resources
        COMPONENT applications
    )
endif()

# 安装配置文件
install(FILES README.md
    DESTINATION share/${PROJECT_NAME}
    COMPONENT applications
)

# 安装许可证
install(FILES LICENSE
    DESTINATION share/${PROJECT_NAME}
    COMPONENT applications
    OPTIONAL
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

# CPack 配置
include(CPack)

# 设置包信息
set(CPACK_PACKAGE_NAME "AsTestTool")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "跨平台设备测试工具")
set(CPACK_PACKAGE_VENDOR "AsTestTool Team")
set(CPACK_PACKAGE_CONTACT "support@astesttool.com")

# 设置包文件
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# Windows 特定配置
if(WIN32)
    # 使用 NSIS 生成器
    set(CPACK_GENERATOR "NSIS")

    # NSIS 特定设置
    set(CPACK_NSIS_DISPLAY_NAME "AsTestTool")
    set(CPACK_NSIS_PACKAGE_NAME "AsTestTool")
    set(CPACK_NSIS_CONTACT "support@astesttool.com")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/astesttool/astesttool")
    set(CPACK_NSIS_HELP_LINK "https://github.com/astesttool/astesttool/wiki")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/astesttool/astesttool")
    set(CPACK_NSIS_MODIFY_PATH ON)

    # 设置安装组件
    set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "AsTestTool Application")
    set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "AsTestTool 主程序")
    set(CPACK_COMPONENT_APPLICATIONS_REQUIRED ON)

    # 设置安装目录
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "AsTestTool")

    # 包含运行时库
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")

    # 创建桌面快捷方式
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\AsTestTool.lnk' '$INSTDIR\\\\bin\\\\AsTestTool.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\AsTestTool.lnk'"
    )

    # 创建开始菜单快捷方式
    set(CPACK_NSIS_MENU_LINKS
        "bin/AsTestTool.exe" "AsTestTool"
    )

elseif(UNIX AND NOT APPLE)
    # Linux 特定配置
    set(CPACK_GENERATOR "TGZ;DEB;RPM")

    # DEB 包配置
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "AsTestTool Team <support@astesttool.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libgl1-mesa-glx")

    # RPM 包配置
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++, mesa-libGL")

elseif(APPLE)
    # macOS 特定配置
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "AsTestTool")
    set(CPACK_DMG_FORMAT "UDZO")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/resources/icons/dmg_background.png")
endif()

# 设置安装组件
set(CPACK_COMPONENTS_ALL applications)

# 设置安装路径
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AsTestTool")

# 包含所有文件
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

# 设置压缩选项
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
