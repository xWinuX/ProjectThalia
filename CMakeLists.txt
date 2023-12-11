cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/AutomateVCPKG.cmake)
include(cmake/Vulkan.cmake)

option(SKIP_AUTOMATE_VCPKG "When ON, you will need to built the packages required on your own or supply your own vcpkg toolchain.")
if (NOT SKIP_AUTOMATE_VCPKG)
    vcpkg_bootstrap()
    vcpkg_install_packages_in_manifest()
endif ()

project(SplitEngine)

add_executable(${PROJECT_NAME}
        main.cpp
        src/SplitEngine/Application.cpp
        include/SplitEngine/Application.hpp
        include/SplitEngine/Window.hpp
        src/SplitEngine/Window.cpp
        src/SplitEngine/Debug/Log.cpp
        include/SplitEngine/Debug/Log.hpp
        src/SplitEngine/ErrorHandler.cpp
        include/SplitEngine/ErrorHandler.hpp
        src/SplitEngine/Rendering/Vulkan/Context.cpp
        include/SplitEngine/Rendering/Vulkan/Context.hpp
        src/SplitEngine/Rendering/Vulkan/Pipeline.cpp
        include/SplitEngine/Rendering/Vulkan/Pipeline.hpp
        src/SplitEngine/IO/Stream.cpp
        include/SplitEngine/IO/Stream.hpp
        src/SplitEngine/Rendering/Vulkan/Swapchain.cpp
        include/SplitEngine/Rendering/Vulkan/Swapchain.hpp
        src/SplitEngine/Rendering/Vulkan/PhysicalDevice.cpp
        include/SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp
        src/SplitEngine/Rendering/Vulkan/Device.cpp
        include/SplitEngine/Rendering/Vulkan/Device.hpp
        src/SplitEngine/Rendering/Vulkan/RenderPass.cpp
        include/SplitEngine/Rendering/Vulkan/RenderPass.hpp
        src/SplitEngine/Rendering/Vulkan/Instance.cpp
        include/SplitEngine/Rendering/Vulkan/Instance.hpp
        src/SplitEngine/Rendering/Renderer.cpp
        include/SplitEngine/Rendering/Renderer.hpp
        include/SplitEngine/Rendering/Vulkan/Utility.hpp
        include/SplitEngine/Event.hpp
        include/SplitEngine/Rendering/Vertex.hpp
        src/SplitEngine/Rendering/Vulkan/Buffer.cpp
        include/SplitEngine/Rendering/Vulkan/Buffer.hpp
        src/SplitEngine/Rendering/Vulkan/DeviceObject.cpp
        include/SplitEngine/Rendering/Vulkan/DeviceObject.hpp
        src/SplitEngine/Rendering/Vulkan/Image.cpp
        include/SplitEngine/Rendering/Vulkan/Image.hpp
        src/SplitEngine/IO/ImageFile.cpp
        include/SplitEngine/IO/ImageFile.hpp
        src/SplitEngine/Rendering/Vulkan/Allocator.cpp
        include/SplitEngine/Rendering/Vulkan/Allocator.hpp
        src/SplitEngine/Rendering/Material.cpp
        include/SplitEngine/Rendering/Material.hpp
        src/SplitEngine/Rendering/Vulkan/DescriptorSetManager.cpp
        include/SplitEngine/Rendering/Vulkan/DescriptorSetManager.hpp
        src/SplitEngine/Input.cpp
        include/SplitEngine/Input.hpp
        src/SplitEngine/Rendering/Vulkan/DescriptorSetManager.cpp
        src/SplitEngine/Rendering/Texture2D.cpp
        include/SplitEngine/Rendering/Texture2D.hpp
        include/SplitEngine/Rendering/TextureSettings.hpp
        src/SplitEngine/Rendering/Material.cpp
        src/SplitEngine/Rendering/Model.cpp
        include/SplitEngine/Rendering/Model.hpp
        src/SplitEngine/Rendering/Shader.cpp
        include/SplitEngine/Rendering/Shader.hpp
        include/SplitEngine/DataStructures.hpp
        src/SplitEngine/Rendering/Texture2D.cpp
        include/SplitEngine/ApplicationInfo.hpp
)

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

# Copy all res files to binary folder
file(COPY res DESTINATION "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

# Compile all shaders
compile_shader(${PROJECT_NAME}
        ENV vulkan1.3
        FORMAT spv
        SOURCES
        /res/shaders/debug/Debug.vert
        /res/shaders/debug/Debug.frag
)
