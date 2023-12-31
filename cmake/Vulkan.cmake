# Source: https://stackoverflow.com/questions/60420700/cmake-invocation-of-glslc-with-respect-to-includes-dependencies

find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT;OUTPUT" "SOURCES")

    # Determine the build configuration folder (debug, Release, etc.)
    set(config_folder "$<CONFIG>")

    message(${arg_SOURCES})

    foreach (source ${arg_SOURCES})
        # Remove absolute path
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" source ${source})

        add_custom_command(
                OUTPUT ${arg_OUTPUT}/${source}.${arg_FORMAT} # Change output path
                DEPENDS ${source}
                DEPFILE ${arg_OUTPUT}/${source}.d
                COMMAND
                glslangValidator
                -e main
                -gVS
                -V
                -o ${arg_OUTPUT}/${source}.${arg_FORMAT}  # Change output path
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
        )

        target_sources(${target} PRIVATE ${arg_OUTPUT}/${source}.${arg_FORMAT})  # Change source path
    endforeach ()
endfunction()