cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_TOOLCHAIN_FILE})

project(ProjectThalia)

add_executable(${PROJECT_NAME}
        main.cpp
        include/ProjectThalia/Core/Window.hpp
        src/ProjectThalia/Core/Window.cpp
        src/ProjectThalia/Core/Application.cpp
        include/ProjectThalia/Core/Application.hpp
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