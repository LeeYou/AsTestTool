cmake_minimum_required(VERSION 3.16)
project(V4L2Plugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/linux)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/linux)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/linux)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/V4L2Plugin.cpp
)

# 头文件
set(HEADERS
    include/V4L2Plugin.h
)

# 创建共享库
add_library(V4L2Plugin SHARED ${SOURCES} ${HEADERS})

# 设置输出名称
set_target_properties(V4L2Plugin PROPERTIES
    OUTPUT_NAME "V4L2Plugin"
    PREFIX "lib"
    SUFFIX ".so"
)

# Linux特定设置
if(UNIX AND NOT APPLE)
    # 链接V4L2库
    target_link_libraries(V4L2Plugin
        pthread
        rt
    )
    
    # 设置编译选项
    target_compile_options(V4L2Plugin PRIVATE
        -Wall -Wextra -Wpedantic
        -fPIC
    )
    
    # 设置链接选项
    target_link_options(V4L2Plugin PRIVATE
        -shared
        -Wl,-soname,libV4L2Plugin.so
    )
    
    # 设置运行时路径
    set_target_properties(V4L2Plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# 编译选项
target_compile_options(V4L2Plugin PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# 安装规则
install(TARGETS V4L2Plugin
    RUNTIME DESTINATION plugins/linux
    LIBRARY DESTINATION plugins/linux
    ARCHIVE DESTINATION plugins/linux
)

# 复制到构建目录
add_custom_command(TARGET V4L2Plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:V4L2Plugin>
    ${CMAKE_BINARY_DIR}/plugins/linux/
    COMMENT "Copying libV4L2Plugin.so to plugins/linux/"
)
