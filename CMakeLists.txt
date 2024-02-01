cmake_minimum_required(VERSION 3.26)

option(HEADLESS "Builds engine without rendering and audio features" OFF)
option(SKIP_AUTOMATE_VCPKG "When ON, you will need to built the packages required on your own or supply your own vcpkg toolchain.")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_compile_definitions(SE_DEBUG_LOG)

if (HEADLESS)
    add_compile_definitions(SE_HEADLESS)
endif ()

project(SplitEngine VERSION 1.0.0)

set(SOLOUD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/soloud/src)
set(SOLOUD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/soloud/include)

set(SOURCE_FILES
        include/SplitEngine/Application.hpp
        include/SplitEngine/ApplicationInfo.hpp
        include/SplitEngine/AssetDatabase.hpp
        include/SplitEngine/DataStructures.hpp
        include/SplitEngine/Debug/Log.hpp
        include/SplitEngine/Debug/Performance.hpp
        include/SplitEngine/ECS/Archetype.hpp
        include/SplitEngine/ECS/Component.hpp
        include/SplitEngine/ECS/Context.hpp
        include/SplitEngine/ECS/Entity.hpp
        include/SplitEngine/ECS/Registry.hpp
        include/SplitEngine/ECS/Stage.hpp
        include/SplitEngine/ECS/System.hpp
        include/SplitEngine/ECS/SystemBase.hpp
        include/SplitEngine/ErrorHandler.hpp
        include/SplitEngine/Event.hpp
        include/SplitEngine/Input.hpp
        include/SplitEngine/IO/Image.hpp
        include/SplitEngine/IO/ImageLoader.hpp
        include/SplitEngine/IO/Stream.hpp
        include/SplitEngine/KeyCode.hpp
        include/SplitEngine/Tools/ImagePacker.hpp
        include/SplitEngine/Tools/ImageSlicer.hpp
        include/SplitEngine/Utility/String.hpp
        src/SplitEngine/Application.cpp
        src/SplitEngine/Debug/Log.cpp
        src/SplitEngine/ECS/Archetype.cpp
        src/SplitEngine/ECS/Registry.cpp
        src/SplitEngine/ErrorHandler.cpp
        src/SplitEngine/Input.cpp
        src/SplitEngine/IO/ImageLoader.cpp
        src/SplitEngine/IO/Stream.cpp
        src/SplitEngine/Tools/ImagePacker.cpp
        src/SplitEngine/Tools/ImageSlicer.cpp
        src/SplitEngine/Utility/String.cpp
)

if (NOT HEADLESS)
    set(NON_HEADLESS_FILES
            include/SplitEngine/Audio/Manager.hpp
            include/SplitEngine/Audio/SoundEffect.hpp
            include/SplitEngine/IO/Audio.hpp
            include/SplitEngine/IO/AudioLoader.hpp
            include/SplitEngine/Rendering/Material.hpp
            include/SplitEngine/Rendering/Model.hpp
            include/SplitEngine/Rendering/Renderer.hpp
            include/SplitEngine/Rendering/Shader.hpp
            include/SplitEngine/Rendering/Sprite.hpp
            include/SplitEngine/Rendering/Texture2D.hpp
            include/SplitEngine/Rendering/TextureSettings.hpp
            include/SplitEngine/Rendering/Vulkan/Allocator.hpp
            include/SplitEngine/Rendering/Vulkan/Buffer.hpp
            include/SplitEngine/Rendering/Vulkan/BufferFactory.hpp
            include/SplitEngine/Rendering/Vulkan/Context.hpp
            include/SplitEngine/Rendering/Vulkan/DescriptorSetAllocator.hpp
            include/SplitEngine/Rendering/Vulkan/Device.hpp
            include/SplitEngine/Rendering/Vulkan/DeviceObject.hpp
            include/SplitEngine/Rendering/Vulkan/Image.hpp
            include/SplitEngine/Rendering/Vulkan/InFlightResource.hpp
            include/SplitEngine/Rendering/Vulkan/Instance.hpp
            include/SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp
            include/SplitEngine/Rendering/Vulkan/Pipeline.hpp
            include/SplitEngine/Rendering/Vulkan/RenderPass.hpp
            include/SplitEngine/Rendering/Vulkan/Swapchain.hpp
            include/SplitEngine/Rendering/Vulkan/Utility.hpp
            include/SplitEngine/Window.hpp
            src/SplitEngine/Audio/Manager.cpp
            src/SplitEngine/Audio/SoundEffect.cpp
            src/SplitEngine/IO/AudioLoader.cpp
            src/SplitEngine/Rendering/Material.cpp
            src/SplitEngine/Rendering/Material.cpp
            src/SplitEngine/Rendering/Model.cpp
            src/SplitEngine/Rendering/Renderer.cpp
            src/SplitEngine/Rendering/Shader.cpp
            src/SplitEngine/Rendering/Sprite.cpp
            src/SplitEngine/Rendering/Texture2D.cpp
            src/SplitEngine/Rendering/Texture2D.cpp
            src/SplitEngine/Rendering/Vulkan/Allocator.cpp
            src/SplitEngine/Rendering/Vulkan/Buffer.cpp
            src/SplitEngine/Rendering/Vulkan/Context.cpp
            src/SplitEngine/Rendering/Vulkan/DescriptorSetAllocator.cpp
            src/SplitEngine/Rendering/Vulkan/DescriptorSetAllocator.cpp
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

    # SOLOUD
    add_compile_definitions(WITH_SDL2)

    file(GLOB_RECURSE
            SOLOUD_SOURCE
            ${SOLOUD_SOURCE_DIR}/backend/sdl/soloud_sdl2_dll.c
            ${SOLOUD_SOURCE_DIR}/backend/sdl/soloud_sdl2.cpp
            ${SOLOUD_SOURCE_DIR}/core/*.cpp
            ${SOLOUD_SOURCE_DIR}/core/*.c
            ${SOLOUD_SOURCE_DIR}/filter/*.cpp
            ${SOLOUD_SOURCE_DIR}/filter/*.c
            ${SOLOUD_SOURCE_DIR}/audiosource/wav/*.c
            ${SOLOUD_SOURCE_DIR}/audiosource/wav/*.cpp
    )
    file(GLOB_RECURSE SOLOUD_INCLUDES ${SOLOUD_INCLUDE_DIR}/*.h ${SOLOUD_INCLUDE_DIR}/*.hpp)

    # Append SOLOUD files to non headless files
    list(APPEND NON_HEADLESS_FILES ${SOLOUD_SOURCE})
    list(APPEND NON_HEADLESS_FILES ${SOLOUD_INCLUDES})

    # Append non headless files to source files
    list(APPEND SOURCE_FILES ${NON_HEADLESS_FILES})
endif ()

add_library(SplitEngine ${SOURCE_FILES})

target_include_directories(SplitEngine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${SOLOUD_INCLUDE_DIR}  # Include SoLoud headers
)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(SplitEngine PUBLIC Vulkan::Vulkan)

# Vulkan Headers
find_package(VulkanHeaders CONFIG)
target_link_libraries(SplitEngine PUBLIC Vulkan::Headers)

# Vulkan Memory Allocator
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC GPUOpen::VulkanMemoryAllocator)

# SPIRV Cross
find_package(spirv_cross_core CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC spirv-cross-core)

find_package(spirv_cross_glsl CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC spirv-cross-glsl)

find_package(spirv_cross_cpp CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC spirv-cross-cpp)

find_package(spirv_cross_reflect CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC spirv-cross-reflect)

find_package(spirv_cross_util CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC spirv-cross-util)

# IMGUI
find_package(imgui CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC imgui::imgui)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(SplitEngine PUBLIC glm::glm)

# SDL 2
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(SplitEngine
        PUBLIC
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# STB
find_package(Stb REQUIRED)
target_include_directories(SplitEngine PRIVATE ${Stb_INCLUDE_DIR})

install(
        TARGETS SplitEngine
        EXPORT SplitEngineTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
        EXPORT SplitEngineTargets
        FILE SplitEngineTargets.cmake
        NAMESPACE SplitEngine::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SplitEngine
)

configure_package_config_file(
        SplitEngineConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/SplitEngineConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SplitEngine
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SplitEngineConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SplitEngine
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/SplitEngineConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
)

install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/SplitEngineConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SplitEngine"
)

install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/SplitEngine"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if (NOT HEADLESS)
    install(
            DIRECTORY ${SOLOUD_INCLUDE_DIR}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif ()