cmake_minimum_required(VERSION 3.26)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

project(macos_native_app)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH "@executable_path")

add_executable(macos_native_app main.mm)

if(OUTPUT_DIR)
  message(STATUS "OUTPUT_DIR: ${OUTPUT_DIR}")
  set(output_dir ${OUTPUT_DIR})
else()
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" system_processor_lower)
  if(system_processor_lower MATCHES "^(x86_64|amd64)$")
    set(targetName "macosX64")
  else()
    set(targetName "macosArm64")
  endif()
  set(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/${targetName}/debugFramework)
  message(STATUS "OUTPUT_DIR is not set. Use default settings: ${output_dir}")
endif()

target_include_directories(macos_native_app PRIVATE ${output_dir}/knative.framework/Headers)
target_link_options(macos_native_app PRIVATE 
  "-F${output_dir}"
  "-framework" 
  "knative"
)
target_link_libraries(macos_native_app PRIVATE
  "-framework Foundation"
)
set_target_properties(macos_native_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_dir})