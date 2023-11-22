# Copyright (C) 2023 by domohuhn
# SPDX-License-Identifier: Zlib

cmake_minimum_required(VERSION 3.14)

project(main VERSION 1.0.0)



option(ENABLE_TRACY_INSTRUMENTATION "Build the library with instrumentation for profiling with tracy" ON)
option(BUILD_TRACY_PROFILER "Build the host application for the tracy profiler" ON)


add_executable(hello main.cpp)



if(ENABLE_TRACY_INSTRUMENTATION OR BUILD_TRACY_PROFILER)
    include(FetchContent)
    set(TRACY_VERSION_TAG v0.10)

    if(UNIX)
        find_program(LSB_RELEASE_EXEC lsb_release)
        execute_process(COMMAND ${LSB_RELEASE_EXEC} -d
            OUTPUT_VARIABLE LSB_RELEASE_ID_DESCRIPTION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(${LSB_RELEASE_ID_DESCRIPTION} MATCHES "Ubuntu 20.04")
            set(TRACY_VERSION_TAG v0.8.1)
            set(USE_OLD_INCLUDE_PATH ON)
        endif()
    endif()
    message(STATUS "Adding tracy ${TRACY_VERSION_TAG}")

    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG        ${TRACY_VERSION_TAG}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    if(ENABLE_TRACY_INTRUMENTATION)
        FetchContent_MakeAvailable(tracy)
    else()
        FetchContent_Populate(tracy)
    endif()
    if(BUILD_TRACY_PROFILER)
        if(WIN32)
            string(SUBSTRING ${TRACY_VERSION_TAG} 1 -1 TRACY_STRIPPED_VERSION_TAG)
            set(TRACY_ARCHIVE_NAME Tracy-${TRACY_STRIPPED_VERSION_TAG}.7z)
            if(NOT "${TRACY_PROFILER_DEPS_VERION_LOCK}" STREQUAL "${TRACY_VERSION_TAG}")
                message(STATUS "Downloading tracy ${TRACY_VERSION_TAG}")
                file(DOWNLOAD https://github.com/wolfpld/tracy/releases/download/${TRACY_VERSION_TAG}/${TRACY_ARCHIVE_NAME} ${CMAKE_BINARY_DIR}/bin/tracy/${TRACY_ARCHIVE_NAME})
                find_program (SEVENZIP_EXECUTABLE 7z.exe)
                message(STATUS "EXEC ${SEVENZIP_EXECUTABLE}")
                if(SEVENZIP_EXECUTABLE)
                    execute_process(COMMAND ${SEVENZIP_EXECUTABLE} x ${CMAKE_BINARY_DIR}/bin/tracy/${TRACY_ARCHIVE_NAME} -o${CMAKE_BINARY_DIR}/bin/tracy/)
                else()
                    message(STATUS "7zip not found - extract ${CMAKE_BINARY_DIR}/bin/tracy/${TRACY_ARCHIVE_NAME} manually or provide the path to 7z via CMAKE_PROGRAM_PATH")
                endif()
                set(TRACY_PROFILER_DEPS_VERION_LOCK ${TRACY_VERSION_TAG} CACHE STRING "downloaded version of tracy" FORCE)
            endif()
        else()
            # you need packages libglfw3-dev libfreetype-dev libcapstone-dev libdbus-dev libtbb-dev
            cmake_host_system_information(RESULT HOST_CORE_COUNT QUERY NUMBER_OF_PHYSICAL_CORES)
            execute_process(COMMAND make -j${HOST_CORE_COUNT}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/_deps/tracy-src/profiler/build/unix
            )
            file(COPY ${CMAKE_BINARY_DIR}/_deps/tracy-src/profiler/build/unix/Tracy-release DESTINATION ${CMAKE_BINARY_DIR}/bin/tracy/)
        endif()
    endif()
endif()

target_link_libraries(hello PRIVATE TracyClient)

configure_file(instrumentation.hpp.in ${CMAKE_BINARY_DIR}/instrumentation.hpp @ONLY)
target_include_directories(hello PRIVATE ${CMAKE_BINARY_DIR})
