cmake_minimum_required(VERSION 3.20.0)

set(TARGET "webview2_composition_sample")
project(${TARGET} VERSION 0.1.0 LANGUAGES C CXX)

# 一定要放在TARGET定义签名，否则不生效
add_compile_options(/utf-8)
add_compile_definitions(_UNICODE UNICODE)

File(GLOB SOURCE_LIST *.cpp *.cc)
add_executable(${TARGET} WIN32 ${SOURCE_LIST})

set(WebView2Path "${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Web.WebView2.1.0.2592.51/build/native")
set(WebView2LibPath "${WebView2Path}/x64")
target_include_directories(${TARGET} PRIVATE "${WebView2Path}/include")
target_link_directories(${TARGET} PRIVATE "${WebView2LibPath}")
target_link_libraries(${TARGET} d3d11 dcomp WebView2Loader.dll.lib)

file(COPY ${WebView2LibPath}/WebView2Loader.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
file(COPY ${WebView2LibPath}/WebView2Loader.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)