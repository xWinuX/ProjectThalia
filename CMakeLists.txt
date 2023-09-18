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

find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SOURCES")

    # Determine the build configuration folder (Debug, Release, etc.)
    set(config_folder "$<CONFIG>")

    foreach(source ${arg_SOURCES})
        add_custom_command(
                OUTPUT ${CMAKE_BINARY_DIR}/bin/${config_folder}/${source}.${arg_FORMAT}  # Change output path
                DEPENDS ${source}
                DEPFILE ${CMAKE_BINARY_DIR}/bin/${config_folder}/${source}.d
                COMMAND
                ${glslc_executable}
                $<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                -MD -MF ${CMAKE_BINARY_DIR}/bin/${config_folder}/${source}.d
                -o ${CMAKE_BINARY_DIR}/bin/${config_folder}/${source}.${arg_FORMAT}  # Change output path
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
        )

        target_sources(${target} PRIVATE ${CMAKE_BINARY_DIR}/bin/${config_folder}/${source}.${arg_FORMAT})  # Change source path
    endforeach()
endfunction()

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
        src/ProjectThalia/VulkanContext.cpp
        include/ProjectThalia/VulkanContext.hpp
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


compile_shader(${PROJECT_NAME}
        ENV vulkan1.2
        FORMAT bin
        SOURCES
        res/shaders/Debug.vert
        res/shaders/Debug.frag
)

# Compile Shaders
#set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders)
#set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/res/shaders)
#
#
#file(GLOB SHADERS
#        ${SHADER_SOURCE_DIR}/*.vert
#        ${SHADER_SOURCE_DIR}/*.frag)
#
#add_custom_command(
#        COMMAND
#        ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
#        OUTPUT ${SHADER_BINARY_DIR}
#        COMMENT "Creating ${SHADER_BINARY_DIR}"
#)
#
#foreach(source IN LISTS SHADERS)
#    get_filename_component(FILENAME ${source} NAME)
#    add_custom_command(
#            COMMAND
#            ${glslc_executable}
#            -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
#            -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
#            ${source}
#            OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
#            DEPENDS ${source} ${SHADER_BINARY_DIR}
#            COMMENT "Compiling ${FILENAME}"
#    )
#    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
#endforeach()
#
#add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})