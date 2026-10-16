cmake_minimum_required(VERSION 3.23 FATAL_ERROR)


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

    include(CMakePackageConfigHelpers)

    set(ROOTHELPER_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    install(
        EXPORT roothelperTargets
        NAMESPACE roothelper::
        DESTINATION ${ROOTHELPER_CMAKE_DIR}
    )

    configure_package_config_file(
        cmake/roothelperConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/roothelperConfig.cmake
        INSTALL_DESTINATION ${ROOTHELPER_CMAKE_DIR}
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/roothelperConfig.cmake
        DESTINATION ${ROOTHELPER_CMAKE_DIR}
    )
endif()
