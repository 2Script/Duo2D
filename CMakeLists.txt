cmake_minimum_required(VERSION 3.15)

project(Duo2D VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set(CPP_FILES
    graphics/pipeline/application.cpp
    graphics/pipeline/image_view.cpp
    graphics/pipeline/instance.cpp
    graphics/pipeline/logical_device.cpp
    graphics/pipeline/physical_device.cpp
    graphics/pipeline/surface.cpp
    graphics/pipeline/swap_chain.cpp
    graphics/pipeline/window.cpp
)
list(TRANSFORM CPP_FILES PREPEND src/Duo2D/)

set(SHADER_INPUT_FILES
    triangle.frag
    triangle.vert
)
list(TRANSFORM SHADER_INPUT_FILES PREPEND shaders/)
list(TRANSFORM SHADER_INPUT_FILES REPLACE "\.[^.]*$" ".hpp" OUTPUT_VARIABLE SHADER_OUTPUT_FILES)
list(TRANSFORM SHADER_OUTPUT_FILES PREPEND ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/)
list(REMOVE_DUPLICATES SHADER_OUTPUT_FILES)

add_library(${PROJECT_NAME} ${CPP_FILES})
add_custom_target(${PROJECT_NAME}_shaders ALL DEPENDS ${SHADER_OUTPUT_FILES} SOURCES ${SHADER_INPUT_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)


## compiler args
list(APPEND CLANG_ARGS "-Wall" "-Wextra" "-fno-exceptions")

target_compile_options(${PROJECT_NAME} PUBLIC ${CLANG_ARGS})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-Werror>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # clang only
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-c++98-compat-pedantic" "-Wc++20-compat-pedantic" "-fsized-deallocation")
else() # gcc only
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wc++20-compat")
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
        COMMAND 
            ${CMAKE_COMMAND}
            -S ${PROJECT_SOURCE_DIR} -B ${PROJECT_BINARY_DIR}
            -DINPUT_SPVS="${COMPILED_SHADER_INPUTS}" -DOUTPUT_HPP="${OUTPUT_HPP}"
            -P ${PROJECT_SOURCE_DIR}/shaders/compile.cmake
    )
endforeach()


### install ###
install(TARGETS ${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY src/ DESTINATION src)

