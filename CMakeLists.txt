cmake_minimum_required(VERSION 3.26)

option(HEADLESS "Builds engine without rendering and audio features" OFF)
option(SKIP_AUTOMATE_VCPKG "When ON, you will need to built the packages required on your own or supply your own vcpkg toolchain.")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(cmake/AutomateVCPKG.cmake)
include(cmake/Vulkan.cmake)

if (NOT SKIP_AUTOMATE_VCPKG)
    vcpkg_bootstrap()
    vcpkg_install_packages_in_manifest()
endif ()

if (HEADLESS)
    add_compile_definitions(SE_HEADLESS)
endif ()

project(SplitEngine)

set(SOURCE_FILES
        include/SplitEngine/Application.hpp
        include/SplitEngine/ApplicationInfo.hpp
        include/SplitEngine/DataStructures.hpp
        include/SplitEngine/Debug/Log.hpp
        include/SplitEngine/ErrorHandler.hpp
        include/SplitEngine/Event.hpp
        include/SplitEngine/IO/ImageFile.hpp
        include/SplitEngine/IO/Stream.hpp
        include/SplitEngine/Input.hpp
        src/SplitEngine/Application.cpp
        src/SplitEngine/Debug/Log.cpp
        src/SplitEngine/ErrorHandler.cpp
        src/SplitEngine/Input.cpp
        src/SplitEngine/IO/ImageFile.cpp
        src/SplitEngine/IO/Stream.cpp
)

if (NOT HEADLESS)
    set(RENDERING_FILES
            include/SplitEngine/Rendering/Material.hpp
            include/SplitEngine/Rendering/Model.hpp
            include/SplitEngine/Rendering/Renderer.hpp
            include/SplitEngine/Rendering/Shader.hpp
            include/SplitEngine/Rendering/Texture2D.hpp
            include/SplitEngine/Rendering/TextureSettings.hpp
            include/SplitEngine/Rendering/Vertex.hpp
            include/SplitEngine/Rendering/Vulkan/Allocator.hpp
            include/SplitEngine/Rendering/Vulkan/Buffer.hpp
            include/SplitEngine/Rendering/Vulkan/Context.hpp
            include/SplitEngine/Rendering/Vulkan/DescriptorSetManager.hpp
            include/SplitEngine/Rendering/Vulkan/Device.hpp
            include/SplitEngine/Rendering/Vulkan/DeviceObject.hpp
            include/SplitEngine/Rendering/Vulkan/Image.hpp
            include/SplitEngine/Rendering/Vulkan/Instance.hpp
            include/SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp
            include/SplitEngine/Rendering/Vulkan/Pipeline.hpp
            include/SplitEngine/Rendering/Vulkan/RenderPass.hpp
            include/SplitEngine/Rendering/Vulkan/Swapchain.hpp
            include/SplitEngine/Rendering/Vulkan/Utility.hpp
            include/SplitEngine/Window.hpp
            src/SplitEngine/Rendering/Material.cpp
            src/SplitEngine/Rendering/Material.cpp
            src/SplitEngine/Rendering/Model.cpp
            src/SplitEngine/Rendering/Renderer.cpp
            src/SplitEngine/Rendering/Shader.cpp
            src/SplitEngine/Rendering/Texture2D.cpp
            src/SplitEngine/Rendering/Texture2D.cpp
            src/SplitEngine/Rendering/Vulkan/Allocator.cpp
            src/SplitEngine/Rendering/Vulkan/Buffer.cpp
            src/SplitEngine/Rendering/Vulkan/Context.cpp
            src/SplitEngine/Rendering/Vulkan/DescriptorSetManager.cpp
            src/SplitEngine/Rendering/Vulkan/DescriptorSetManager.cpp
            src/SplitEngine/Rendering/Vulkan/Device.cpp
            src/SplitEngine/Rendering/Vulkan/DeviceObject.cpp
            src/SplitEngine/Rendering/Vulkan/Image.cpp
            src/SplitEngine/Rendering/Vulkan/Instance.cpp
            src/SplitEngine/Rendering/Vulkan/PhysicalDevice.cpp
            src/SplitEngine/Rendering/Vulkan/Pipeline.cpp
            src/SplitEngine/Rendering/Vulkan/RenderPass.cpp
            src/SplitEngine/Rendering/Vulkan/Swapchain.cpp
            src/SplitEngine/Window.cpp
    )
    list(APPEND SOURCE_FILES ${RENDERING_FILES})
endif ()

add_library(${PROJECT_NAME} ${SOURCE_FILES})

# add everything in include as an... well... include
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Vulkan Headers
find_package(VulkanHeaders CONFIG)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Headers)

# Vulkan Memory Allocator
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Headers GPUOpen::VulkanMemoryAllocator)

# SPIRV Cross
find_package(spirv_cross_core CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-core)

find_package(spirv_cross_glsl CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-glsl)

find_package(spirv_cross_cpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-cpp)


find_package(spirv_cross_reflect CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-reflect)

find_package(spirv_cross_util CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-util)

# IMGUI
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# SDL 2
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# STB
find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})
