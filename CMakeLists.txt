cmake_minimum_required (VERSION 3.12)

project ("LearnVulkan")

set (CMAKE_CXX_STANDARD 17)

# 设置 Vulkan SDK 路径（使用环境变量或硬编码）
if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_DIR "$ENV{VULKAN_SDK}")
else()
    set(VULKAN_SDK_DIR "C:/VulkanSDK/1.4.313.0")
endif()

message(STATUS "Using Vulkan SDK at: ${VULKAN_SDK_DIR}")

include_directories(
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/Include
    SYSTEM ${VULKAN_SDK_DIR}/Include
)

link_directories(
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/Lib
    SYSTEM ${VULKAN_SDK_DIR}/Lib
)

aux_source_directory (. DIRSRCS)

add_subdirectory(vulkanWrapper)
add_subdirectory(texture)

# 将源代码添加到此项目的可执行文件。
add_executable (LearnVulkan ${DIRSRCS})

target_link_libraries(LearnVulkan vulkanLib textureLib vulkan-1.lib glfw3.lib)
