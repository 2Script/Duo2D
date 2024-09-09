cmake_minimum_required(VERSION 3.15)

project(Duo2D VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


set(CPP_FILES
    device.cpp
    application.cpp
    window.cpp
)
list(TRANSFORM CPP_FILES PREPEND src/Duo2D/)

add_library(${PROJECT_NAME} ${CPP_FILES})


## compiler args
list(APPEND CLANG_ARGS "-Wall" "-Wextra" "-fno-exceptions")

target_compile_options(${PROJECT_NAME} PUBLIC ${CLANG_ARGS})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-Werror>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # clang only
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-c++98-compat-pedantic" "-Wc++17-compat-pedantic" "-fsized-deallocation")
else() # gcc only
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wc++17-compat")
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



### install ###
install(TARGETS ${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY src/ DESTINATION src)

