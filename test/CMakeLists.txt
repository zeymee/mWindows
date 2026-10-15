cmake_minimum_required(VERSION 3.20)
project(mWindowsTests LANGUAGES CXX)

include(CTest)

set(MWINDOWS_TEST_CONFIGURATION "${MWINDOWS_TEST_CONFIGURATION}" CACHE STRING
    "Build configuration for single-configuration generators."
)
set_property(CACHE MWINDOWS_TEST_CONFIGURATION PROPERTY STRINGS Debug Release)

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "${MWINDOWS_TEST_CONFIGURATION}" CACHE STRING
        "Build type for single-configuration generators." FORCE
    )
endif()

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_BINARY_DIR}/mWindows"
)

add_executable(mWindows_tests test.cpp)
target_link_libraries(mWindows_tests PRIVATE mWindows)

set(MWINDOWS_TEST_TARGETS mWindows mWindows_tests)

if(MSVC)
    foreach(target IN LISTS MWINDOWS_TEST_TARGETS)
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:/Zi>
            $<$<CONFIG:Debug>:/Od>
            $<$<CONFIG:Debug>:/fsanitize=address>
            $<$<CONFIG:Release>:/O2>
        )
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target IN LISTS MWINDOWS_TEST_TARGETS)
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:-g>
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
            $<$<CONFIG:Debug>:-fsanitize=address>
            $<$<CONFIG:Release>:-O2>
        )
        target_link_options(${target} PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address>
        )
    endforeach()
else()
    message(FATAL_ERROR
        "AddressSanitizer is only configured for MSVC, GNU, and Clang compilers."
    )
endif()

if(BUILD_TESTING)
    add_test(NAME mWindows_tests COMMAND mWindows_tests)

    if(MSVC)
        get_filename_component(MWINDOWS_ASAN_RUNTIME_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(MWINDOWS_ASAN_TEST_PATH "PATH=${MWINDOWS_ASAN_RUNTIME_DIR}\\;$ENV{PATH}")
        set_tests_properties(mWindows_tests PROPERTIES
            ENVIRONMENT "$<$<CONFIG:Debug>:${MWINDOWS_ASAN_TEST_PATH}>"
        )

        add_custom_command(TARGET mWindows_tests POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                "$<IF:$<CONFIG:Debug>,copy_if_different,echo>"
                "${MWINDOWS_ASAN_RUNTIME_DIR}/clang_rt.asan_dynamic-x86_64.dll"
                "$<TARGET_FILE_DIR:mWindows_tests>"
        )
    endif()
endif()