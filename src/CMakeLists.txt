cmake_minimum_required(VERSION 3.15 FATAL_ERROR)


project(roothelper)


set(CMAKE_CXX_STANDARD 17)


option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")


option(DOCS_ONLY "Build documentation only" OFF)

if(DOCS_ONLY)
    add_subdirectory(document)
else()
    find_package(ROOT REQUIRED COMPONENTS Core Hist Gpad)
    add_subdirectory(document)
    add_subdirectory(core)
    add_subdirectory(exec)
endif()
