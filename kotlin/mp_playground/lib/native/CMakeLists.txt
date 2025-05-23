cmake_minimum_required(VERSION 3.15)
project(native_app)

add_executable(native_app main.cc)

if(OUTPUT_DIR)
  message(STATUS "OUTPUT_DIR: ${OUTPUT_DIR}")
  set(output_dir ${OUTPUT_DIR})
else()
  message(STATUS "OUTPUT_DIR is not set. Use default settings.")
  if (WIN32)
    set(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/mingwX64/debugShared)
  else ()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" system_processor_lower)
    if(system_processor_lower MATCHES "^(x86_64|amd64)$")
      set(targetName "macosX64")
    else()
      set(targetName "macosArm64")
    endif()
    set(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/${targetName}/debugShared)
  endif()
endif()

target_include_directories(native_app PRIVATE ${output_dir})
target_link_directories(native_app PRIVATE ${output_dir})
target_link_libraries(native_app PRIVATE knative)
set_target_properties(native_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${output_dir})
set_target_properties(native_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${output_dir})
set_target_properties(native_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_dir})