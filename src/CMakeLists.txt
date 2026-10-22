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

# Usage documents, readable with only the installation present.
install(
    FILES ${PROJECT_SOURCE_DIR}/../README.md
          ${PROJECT_SOURCE_DIR}/../doc/command.html
          ${PROJECT_SOURCE_DIR}/document/API_REFERENCE.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

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

    # Shell and ROOT startup templates for this prefix, and a summary of the one-time user
    # setup printed at the end of the installation.
    install(CODE "
        set(ROOTHELPER_PREFIX \"\${CMAKE_INSTALL_PREFIX}\")
        set(ROOTHELPER_INCLUDEDIR \"${CMAKE_INSTALL_INCLUDEDIR}\")
        set(setup_dir \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/setup\")
        foreach(name bashrc rootlogon.C)
            message(STATUS \"Installing: \${setup_dir}/\${name}\")
            configure_file(\"${PROJECT_SOURCE_DIR}/cmake/\${name}.in\"
                           \"\$ENV{DESTDIR}\${setup_dir}/\${name}\" @ONLY)
        endforeach()
        set(font_conf \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/fontconfig/99-roothelper-fonts.conf\")

        # Mark the steps that the current user has already done.
        set(bashrc_done \"\")
        if(EXISTS \"\$ENV{HOME}/.bashrc\")
            file(READ \"\$ENV{HOME}/.bashrc\" text)
            # Also accept the prefix written with $HOME.
            string(REPLACE \"\$ENV{HOME}/\" \"\\\$HOME/\" home_prefix \"\${ROOTHELPER_PREFIX}\")
            string(FIND \"\${text}\" \"\${ROOTHELPER_PREFIX}/bin\" found)
            string(FIND \"\${text}\" \"\${home_prefix}/bin\" found_home)
            if(NOT found EQUAL -1 OR NOT found_home EQUAL -1)
                set(bashrc_done \"  (done)\")
            endif()
        endif()
        set(rootlogon_done \"\")
        if(EXISTS \"\$ENV{HOME}/.rootlogon.C\")
            file(READ \"\$ENV{HOME}/.rootlogon.C\" text)
            string(FIND \"\${text}\" \"roothelper\" found)
            if(NOT found EQUAL -1)
                set(rootlogon_done \"  (done)\")
            endif()
        endif()
        set(font_done \"\")
        set(font_link \"\$ENV{HOME}/.config/fontconfig/conf.d/99-roothelper-fonts.conf\")
        if(IS_SYMLINK \"\${font_link}\")
            file(READ_SYMLINK \"\${font_link}\" target)
            if(target STREQUAL font_conf)
                set(font_done \"  (done)\")
            endif()
        endif()
        message(\"
roothelper is installed in \${ROOTHELPER_PREFIX}.
Documents (README, API reference, commands): \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}
One-time setup for each user:
  1. Shell (PATH, ROOT_INCLUDE_PATH, CMAKE_PREFIX_PATH)\${bashrc_done}:
       cat \${setup_dir}/bashrc >> ~/.bashrc
  2. ROOT startup, needed only for ACLiC (macro.C+)\${rootlogon_done}:
       cp \${setup_dir}/rootlogon.C ~/.rootlogon.C
  3. Base 14 substitute fonts for Inkscape and PDF viewers\${font_done}:
       mkdir -p ~/.config/fontconfig/conf.d && ln -sf \${font_conf} ~/.config/fontconfig/conf.d/
\")
    ")
endif()
