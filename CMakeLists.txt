cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/AutomateVCPKG.cmake)
include(cmake/Vulkan.cmake)

option(SKIP_AUTOMATE_VCPKG "When ON, you will need to built the packages required by MyProject on your own or supply your own vcpkg toolchain.")
if (NOT SKIP_AUTOMATE_VCPKG)
    vcpkg_bootstrap()
    vcpkg_install_packages_in_manifest()
endif ()

project(ProjectThalia)

add_executable(${PROJECT_NAME}
        main.cpp
        src/ProjectThalia/Application.cpp
        include/ProjectThalia/Application.hpp
        include/ProjectThalia/Window.hpp
        src/ProjectThalia/Window.cpp
        src/ProjectThalia/Debug/Log.cpp
        include/ProjectThalia/Debug/Log.hpp
        src/ProjectThalia/ErrorHandler.cpp
        include/ProjectThalia/ErrorHandler.hpp
        src/ProjectThalia/Rendering/Vulkan/Context.cpp
        include/ProjectThalia/Rendering/Vulkan/Context.hpp
        src/ProjectThalia/Rendering/Vulkan/Pipeline.cpp
        include/ProjectThalia/Rendering/Vulkan/Pipeline.hpp
        src/ProjectThalia/IO/Stream.cpp
        include/ProjectThalia/IO/Stream.hpp
        src/ProjectThalia/Rendering/Vulkan/Swapchain.cpp
        include/ProjectThalia/Rendering/Vulkan/Swapchain.hpp
        src/ProjectThalia/Rendering/Vulkan/PhysicalDevice.cpp
        include/ProjectThalia/Rendering/Vulkan/PhysicalDevice.hpp
        src/ProjectThalia/Rendering/Vulkan/Device.cpp
        include/ProjectThalia/Rendering/Vulkan/Device.hpp
        src/ProjectThalia/Rendering/Vulkan/RenderPass.cpp
        include/ProjectThalia/Rendering/Vulkan/RenderPass.hpp
        src/ProjectThalia/Rendering/Vulkan/Instance.cpp
        include/ProjectThalia/Rendering/Vulkan/Instance.hpp
        src/ProjectThalia/Rendering/Renderer.cpp
        include/ProjectThalia/Rendering/Renderer.hpp
        include/ProjectThalia/Rendering/Vulkan/Utility.hpp
        include/ProjectThalia/Event.hpp
        include/ProjectThalia/Rendering/Vertex.hpp
        src/ProjectThalia/Rendering/Vulkan/Buffer.cpp
        include/ProjectThalia/Rendering/Vulkan/Buffer.hpp
        src/ProjectThalia/Rendering/Vulkan/DeviceObject.cpp
        include/ProjectThalia/Rendering/Vulkan/DeviceObject.hpp
        src/ProjectThalia/Rendering/Vulkan/Image.cpp
        include/ProjectThalia/Rendering/Vulkan/Image.hpp
        src/ProjectThalia/IO/ImageFile.cpp
        include/ProjectThalia/IO/ImageFile.hpp
        src/ProjectThalia/Rendering/Vulkan/Allocator.cpp
        include/ProjectThalia/Rendering/Vulkan/Allocator.hpp
        include/ProjectThalia/Rendering/Vulkan/DescriptorPool.cpp
        include/ProjectThalia/Rendering/Vulkan/DescriptorPool.hpp
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
        /res/shaders/Debug.vert
        /res/shaders/Debug.frag
)
