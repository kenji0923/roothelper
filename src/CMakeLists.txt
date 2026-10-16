cmake_minimum_required(VERSION 3.23 FATAL_ERROR)


project(roothelper)


set(CMAKE_CXX_STANDARD 17)


option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")


option(DOCS_ONLY "Build documentation only" OFF)


# Helper scripts (../script) are installed as executables to <prefix>/bin.
include(GNUInstallDirs)
file(GLOB ROOTHELPER_SCRIPTS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/../script/*)
install(PROGRAMS ${ROOTHELPER_SCRIPTS} DESTINATION ${CMAKE_INSTALL_BINDIR})

# Substitutes for the PDF base 14 fonts (../font) and a fontconfig file that uses them in
# place of Helvetica, Times, Courier, Symbol and Zapf Dingbats. The file is written at
# installation time so that "cmake --install --prefix" gives the right font directory.
set(ROOTHELPER_FONT_DIR ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/fonts)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/../font/ DESTINATION ${ROOTHELPER_FONT_DIR})
install(CODE "
    set(ROOTHELPER_FONT_DIR \"\${CMAKE_INSTALL_PREFIX}/${ROOTHELPER_FONT_DIR}\")
    set(conf \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/fontconfig/99-roothelper-fonts.conf\")
    message(STATUS \"Installing: \${conf}\")
    configure_file(\"${PROJECT_SOURCE_DIR}/cmake/roothelper-fonts.conf.in\" \"\${conf}\" @ONLY)
")

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
