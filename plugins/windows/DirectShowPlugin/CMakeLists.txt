cmake_minimum_required(VERSION 3.16)
project(DirectShowPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/windows)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/windows)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/windows)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/DirectShowPlugin.cpp
)

# 头文件
set(HEADERS
    include/DirectShowPlugin.h
)

# 创建共享库
add_library(DirectShowPlugin SHARED ${SOURCES} ${HEADERS})

# 设置输出名称
set_target_properties(DirectShowPlugin PROPERTIES
    OUTPUT_NAME "DirectShowPlugin"
    PREFIX ""
    SUFFIX ".dll"
)

# Windows特定设置
if(WIN32)
    # 链接DirectShow库和Logger库
    target_link_libraries(DirectShowPlugin
        ole32
        oleaut32
        strmiids
        quartz
        Logger
    )
    
    # 设置DLL导出
    target_compile_definitions(DirectShowPlugin PRIVATE
        DIRECTSHOW_PLUGIN_EXPORTS
        ENABLE_LOGGING
    )
    
    # 设置运行时库
    set_property(TARGET DirectShowPlugin PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# 编译选项
target_compile_options(DirectShowPlugin PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# 安装规则
install(TARGETS DirectShowPlugin
    RUNTIME DESTINATION plugins/windows
    LIBRARY DESTINATION plugins/windows
    ARCHIVE DESTINATION plugins/windows
)

# 复制到构建目录
add_custom_command(TARGET DirectShowPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:DirectShowPlugin>
    ${CMAKE_BINARY_DIR}/plugins/windows/
    COMMENT "Copying DirectShowPlugin.dll to plugins/windows/"
)
