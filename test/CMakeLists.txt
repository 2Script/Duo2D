cmake_minimum_required(VERSION 3.15)

set(TARGETS arithmetic_types arithmetic_ops application)
set(SANITIZERS undefined)

list(TRANSFORM TARGETS PREPEND "test_" OUTPUT_VARIABLE TARGET_LIST)
foreach(TEST_TARGET IN LISTS TARGET_LIST)
    add_executable(${TEST_TARGET} "${TEST_TARGET}.cpp")
    add_dependencies(${TEST_TARGET} Duo2D)

    #target_compile_options(${TEST_TARGET} PUBLIC "-march=native")
    target_compile_options(${TEST_TARGET} PUBLIC "-fno-rtti" "-fno-exceptions" "-fopenmp" "-Wall" "-Wextra" "$<$<CONFIG:RELEASE>:-Werror>")
    target_compile_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:RELEASE>:-Werror>" "$<$<CONFIG:RELEASE>:-O3>" "$<$<CONFIG:RELEASE>:-ffast-math>" "$<$<CONFIG:RELEASE>:-march=native>")
    target_compile_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:DEBUG>:-O0>" "$<$<CONFIG:DEBUG>:-g>")
   
    #target_link_options(${TEST_TARGET} PUBLIC "-fuse-ld=mold")
    target_link_options(${TEST_TARGET} PUBLIC "-v")

    target_link_directories(${TEST_TARGET} PUBLIC "/usr/local/lib" "/usr/lib")
    target_link_libraries(${TEST_TARGET} PUBLIC Duo2D)
    target_link_libraries(${TEST_TARGET} PUBLIC glfw)
    target_link_libraries(${TEST_TARGET} PUBLIC vulkan)
    target_link_libraries(${TEST_TARGET} PUBLIC DuoDecode)
    target_link_libraries(${TEST_TARGET} PUBLIC avdevice)
    target_link_libraries(${TEST_TARGET} PUBLIC avformat)
    target_link_libraries(${TEST_TARGET} PUBLIC avcodec)
    target_link_libraries(${TEST_TARGET} PUBLIC avutil)
    target_link_libraries(${TEST_TARGET} PUBLIC llfio)
    target_link_libraries(${TEST_TARGET} PUBLIC msdfgen-core)
    target_link_libraries(${TEST_TARGET} PUBLIC msdfgen-ext)
    target_link_libraries(${TEST_TARGET} PUBLIC harfbuzz)

    foreach(SAN IN LISTS SANITIZERS)
        target_compile_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:DEBUG>:-fsanitize=${SAN}>")
        target_link_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:DEBUG>:-fsanitize=${SAN}>")
    endforeach()


    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${TEST_TARGET} PUBLIC "-fsized-deallocation")
    endif() 
endforeach()