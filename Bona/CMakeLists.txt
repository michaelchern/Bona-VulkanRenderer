cmake_minimum_required (VERSION 3.12)

project ("Bona")

set (CMAKE_CXX_STANDARD 17)

file(GLOB ASSETS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

file(COPY ${ASSETS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_DIR "$ENV{VULKAN_SDK}")
else()
    set(VULKAN_SDK_DIR "C:/VulkanSDK/1.4.313.0")
endif()

message(STATUS "Using Vulkan SDK at: ${VULKAN_SDK_DIR}")

include_directories(
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Include
    SYSTEM ${VULKAN_SDK_DIR}/Include)

link_directories(
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Lib
    SYSTEM ${VULKAN_SDK_DIR}/Lib)

aux_source_directory (. DIRSRCS)

add_subdirectory(vulkanWrapper)
add_subdirectory(texture)

add_executable (Bona ${DIRSRCS})

target_link_libraries(Bona vulkanLib textureLib vulkan-1.lib glfw3.lib)
