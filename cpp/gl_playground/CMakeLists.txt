cmake_minimum_required(VERSION 3.0.0)
project(gl_playground VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# 环境搭建：https://cloud.tencent.com/developer/article/2071165

if (WIN32)
set(GLFW_HOME "./glfw-3.3.9.bin.WIN64")
link_directories("${GLFW_HOME}/lib-mingw-w64")
set(GL_LIB "")
set(GLFW_LIB_NAME "glfw3dll")
file(COPY "./glfw-3.3.9.bin.WIN64/lib-mingw-w64/glfw3.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
elseif (APPLE)
set(GLFW_HOME "./glfw-3.3.9.bin.MACOS")
link_directories("${GLFW_HOME}/lib-universal")
set(GL_LIB "-framework OpenGL")
set(GLFW_LIB_NAME "glfw.3")
file(COPY "./glfw-3.3.9.bin.MACOS/lib-universal/libglfw.3.dylib" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else ()
MESSAGE(FATAL_ERROR "Not supported platform.")
endif ()
set(GLAD_HOME "./glad")

# 设置头文件目录
include_directories("${GLFW_HOME}/include")
include_directories("${GLAD_HOME}/include")

# 编译 GLAD 库
add_library(glad SHARED "${GLAD_HOME}/src/glad.c")

# 创建可执行文件
set(SRC_FILES main.cpp)
if (APPLE)
add_executable(gl_playground MACOSX_BUNDLE ${SRC_FILES})
set_target_properties(gl_playground PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER com.yinzhou.glplayground
    MACOSX_BUNDLE_BUNDLE_NAME gl_playground
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_INFO_PLIST "./MacOSXBundleInfo.plist.in"
)
else ()
add_executable(gl_playground ${SRC_FILES})
endif()


# 链接 GLFW GLAD OpenGL
target_link_libraries(gl_playground ${GLFW_LIB_NAME} glad ${GL_LIB})
