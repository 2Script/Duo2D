cmake_minimum_required(VERSION 3.15)

project(Duo2D VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

set(CPP_FILES
    core/application.cpp

    vulkan/core/command_buffer.cpp
    vulkan/core/command_pool.cpp
    vulkan/core/instance.cpp

    vulkan/device/logical_device.cpp
    vulkan/device/physical_device.cpp

    vulkan/display/framebuffer.cpp
    vulkan/display/image_sampler.cpp
    vulkan/display/image_view.cpp
    vulkan/display/render_pass.cpp
    vulkan/display/surface.cpp
    vulkan/display/swap_chain.cpp
    vulkan/display/texture.cpp
    
    vulkan/memory/buffer.cpp
    vulkan/memory/image.cpp

    vulkan/sync/semaphore.cpp
    vulkan/sync/fence.cpp
)
list(TRANSFORM CPP_FILES PREPEND src/Duo2D/)

set(SHADER_INPUT_FILES
    rect.vert
    rect.frag
    glyph.vert
    glyph.frag
    debug_rect.vert
    debug_rect.frag
)
list(TRANSFORM SHADER_INPUT_FILES PREPEND shaders/)
list(TRANSFORM SHADER_INPUT_FILES REPLACE "\.[^.]*$" ".hpp" OUTPUT_VARIABLE SHADER_OUTPUT_FILES)
list(TRANSFORM SHADER_OUTPUT_FILES PREPEND ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/)
list(REMOVE_DUPLICATES SHADER_OUTPUT_FILES)

add_library(${PROJECT_NAME} ${CPP_FILES})
add_custom_target(${PROJECT_NAME}_shaders ALL DEPENDS ${SHADER_OUTPUT_FILES} SOURCES ${SHADER_INPUT_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)

## custom includes
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(FT freetype2)
#target_include_directories(${PROJECT_NAME} PUBLIC ${FT_INCLUDE_DIRS})


## compiler args
list(APPEND CLANG_ARGS "-Wall" "-Wextra" "-fno-exceptions" "-fno-rtti" "-fopenmp")

target_compile_options(${PROJECT_NAME} PUBLIC ${CLANG_ARGS})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-Werror>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-g>")
target_link_options(${PROJECT_NAME} PUBLIC "-fopenmp")
#target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3>")
#target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-ffast-math>")
#target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-march=native>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # clang only
    #target_compile_options(${PROJECT_NAME} PUBLIC "-Wc++20-compat-pedantic")
    target_compile_options(${PROJECT_NAME} PUBLIC "-fsized-deallocation")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.1.0")
        # Don't show warnings for C++20 designated field initializers.
        target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-missing-designated-field-initializers")
    else()
        # If clang is "outdated", then we have to disable the entire warning category
        target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-missing-field-initializers")
    endif()
else() # gcc only
    #target_compile_options(${PROJECT_NAME} PUBLIC "-Wc++20-compat")
    # gcc has no "missing-designated-field-initializers" (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96868)
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-missing-field-initializers")
endif()


get_target_property(COMPILER_FLAGS ${PROJECT_NAME} COMPILE_OPTIONS)
message(STATUS "Compiler options: ${COMPILER_FLAGS}")



### include directory ###
cmake_path(SET SELF_INC_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(TO_NATIVE_PATH "${SELF_INC_CMAKE_DIR}" SELF_INC_DIR)
target_include_directories(${PROJECT_NAME} BEFORE PUBLIC "${SELF_INC_DIR}")
message(STATUS "Including self directory: ${SELF_INC_DIR}")



### export header ###
# include(GenerateExportHeader)
# generate_export_header(${PROJECT_NAME} BASE_NAME XSONPP EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/xsonpp/export_defs.h")



### clang compile commands ###
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


### shader compiliation ###
foreach(shader ${SHADER_INPUT_FILES})
    set(shader_in ${PROJECT_SOURCE_DIR}/${shader})
    set(spv_out ${shader}.spv)
    set(dep_file ${shader}.d)

    get_filename_component(SHADER_NAME ${shader} NAME_WE)
    list(APPEND SHADER_INPUT_FILES_${SHADER_NAME} ${PROJECT_BINARY_DIR}/${shader})
    
    add_custom_command(
        OUTPUT ${spv_out}
        MAIN_DEPENDENCY ${shader_in}
        DEPFILE ${dep_file}
        COMMENT "Compiling SPIRV shader file ${PROJECT_BINARY_DIR}/${spv_out}"
        COMMAND
            glslc ${shader_in}
            -MD -MF ${dep_file}
            -o ${spv_out}
    )
endforeach()

foreach(OUTPUT_HPP ${SHADER_OUTPUT_FILES})
    get_filename_component(SHADER_NAME ${OUTPUT_HPP} NAME_WE)
    string(REPLACE ";" "$<SEMICOLON>" COMPILED_SHADER_INPUTS "${SHADER_INPUT_FILES_${SHADER_NAME}}")
    list(TRANSFORM SHADER_INPUT_FILES_${SHADER_NAME} APPEND .spv OUTPUT_VARIABLE COMPILED_SHADER_FILES)
    string(REPLACE ";" "$<SEMICOLON>" COMPILED_SHADER_FILES "${COMPILED_SHADER_FILES}")

    add_custom_command(
        OUTPUT ${OUTPUT_HPP}
        DEPENDS ${COMPILED_SHADER_FILES}
        COMMENT "Generating shader header ${OUTPUT_HPP}"
        COMMAND ${CMAKE_COMMAND} -E echo "//Auto-generated by cmake" > ${OUTPUT_HPP}
        COMMAND ${CMAKE_COMMAND} -E echo "\\#pragma once" >> ${OUTPUT_HPP}
        COMMAND ${CMAKE_COMMAND} -E echo "namespace d2d::shaders {" >> ${OUTPUT_HPP}
        COMMAND ${CMAKE_COMMAND} -E echo "struct ${SHADER_NAME} {" >> ${OUTPUT_HPP}
        COMMAND 
            ${CMAKE_COMMAND}
            -S ${PROJECT_SOURCE_DIR} -B ${PROJECT_BINARY_DIR}
            -DINPUT_SPVS="${COMPILED_SHADER_INPUTS}" -DOUTPUT_HPP="${OUTPUT_HPP}"
            -P ${PROJECT_SOURCE_DIR}/shaders/compile.cmake
        COMMAND ${CMAKE_COMMAND} -E echo "}\;" >> ${OUTPUT_HPP}
        COMMAND ${CMAKE_COMMAND} -E echo "}" >> ${OUTPUT_HPP}
    )
endforeach()


### test executables ###
add_subdirectory(test)

### install ###
##TODO: install debug and release
install(TARGETS ${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY src/ DESTINATION src)

