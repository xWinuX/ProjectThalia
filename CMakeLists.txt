cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(SKIP_AUTOMATE_VCPKG "When ON, you will need to built the packages required by MyProject on your own or supply your own vcpkg toolchain.")
if (NOT SKIP_AUTOMATE_VCPKG)
    include(cmake/AutomateVCPKG.cmake)

    vcpkg_bootstrap()
    vcpkg_install_packages_in_manifest()
endif()

include(cmake/Vulkan.cmake)

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
        src/ProjectThalia/Rendering/VulkanContext.cpp
        include/ProjectThalia/Rendering/VulkanContext.hpp
        src/ProjectThalia/Rendering/Pipeline.cpp
        include/ProjectThalia/Rendering/Pipeline.hpp
        include/ProjectThalia/IO/Stream.cpp
        include/ProjectThalia/IO/Stream.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Vulkan Headers
find_package(VulkanHeaders CONFIG)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Headers)

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

file(GLOB_RECURSE SHADERS res/shaders/*.vert res/shaders/*.frag)

message(${SHADERS})

compile_shader(${PROJECT_NAME}
        ENV vulkan1.2
        FORMAT spv
        SOURCES ${SHADERS}
)
