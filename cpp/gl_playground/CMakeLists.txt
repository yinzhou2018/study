cmake_minimum_required(VERSION 3.20.0)
project(gl_playground VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# 环境搭建：https://cloud.tencent.com/developer/article/2071165

file(GLOB PNG_SOURCES "*.png")
file(GLOB JPG_SOURCES "*.jpg")
list(APPEND IMG_SOURCES ${PNG_SOURCES} ${JPG_SOURCES})

if (WIN32)
add_compile_options(/utf-8)
set(GLFW_HOME "./glfw-3.3.9.bin.WIN64")
link_directories("${GLFW_HOME}/lib-vc2022")
set(GL_LIB "")
set(GLFW_LIB_NAME "glfw3dll")
file(COPY "./glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
file(COPY "./glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")

foreach(FILE ${IMG_SOURCES})
    file(COPY ${FILE} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
    file(COPY ${FILE} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
endforeach()

elseif (APPLE)
set(GLFW_HOME "./glfw-3.3.9.bin.MACOS")
link_directories("${GLFW_HOME}/lib-universal")
set(GL_LIB "-framework OpenGL")
set(GLFW_LIB_NAME "glfw.3")
file(COPY "./glfw-3.3.9.bin.MACOS/lib-universal/libglfw.3.dylib" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
foreach(FILE ${IMG_SOURCES})
    file(COPY ${FILE} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endforeach()
else ()
MESSAGE(FATAL_ERROR "Not supported platform.")
endif ()
set(GLAD_HOME "./glad")

# 设置头文件目录
include_directories("${GLFW_HOME}/include")
include_directories("${GLAD_HOME}/include")

# 编译 GLAD 库
add_library(glad SHARED "${GLAD_HOME}/src/glad.c")
target_compile_definitions(glad PUBLIC GLAD_GLAPI_EXPORT)
target_compile_definitions(glad PRIVATE GLAD_GLAPI_EXPORT_BUILD)

# 创建可执行文件
file(GLOB SRC_FILES "*.cc" "*.h" "*.cpp")
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
