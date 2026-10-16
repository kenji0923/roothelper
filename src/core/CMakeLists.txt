include(GNUInstallDirs)


add_library(DataSaver data_saver.cpp)
target_include_directories(DataSaver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(DataSaver PUBLIC
    ROOT::Gpad
)


add_library(Graphics graphics.cpp)
target_include_directories(Graphics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(Graphics PUBLIC
    ROOT::Gpad
)


add_library(Container container.cpp)
target_include_directories(Container PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(Container PUBLIC
    ROOT::Gpad
    Graphics
)


add_library(Analysis analysis.cpp)
target_include_directories(Analysis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(Analysis PUBLIC
    ROOT::Hist
)


add_library(RootHelper INTERFACE)
target_include_directories(RootHelper INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(RootHelper INTERFACE
    DataSaver
    Graphics
    Container
    Analysis
)
target_sources(RootHelper PUBLIC
    FILE_SET header
    TYPE HEADERS
    BASE_DIRS include/roothelper
    FILES
	include/roothelper/data_saver.h
	include/roothelper/graphics.h
	include/roothelper/container.h
	include/roothelper/analysis.h
	include/roothelper/roothelper.h
)
target_sources(RootHelper PUBLIC
    FILE_SET top_header_for_interpreter
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../roothelper_source.h
)
target_sources(RootHelper PUBLIC
    FILE_SET source_for_interpreter
    TYPE HEADERS
    FILES
	data_saver.cpp
	graphics.cpp
	container.cpp
	analysis.cpp
)


# Distinct file names avoid clashes with other libraries (e.g. a generic libGraphics.so).
# $ORIGIN lets each installed library find the others in the same directory.
foreach(target DataSaver Graphics Container Analysis)
    string(REGEX REPLACE "([a-z])([A-Z])" "\\1_\\2" snake_name ${target})
    string(TOLOWER ${snake_name} snake_name)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME roothelper_${snake_name}
        INSTALL_RPATH "$ORIGIN"
    )
endforeach()


install(
    TARGETS
	DataSaver
	Graphics
	Container
	Analysis
	RootHelper
    EXPORT roothelperTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET header DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILE_SET top_header_for_interpreter DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILE_SET source_for_interpreter DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/src
)


# Header that loads the installed libraries into the ROOT interpreter. It is written at
# installation time so that "cmake --install --prefix" gives the right paths.
set(ROOTHELPER_CLING_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cling_libraries.h.in)
if(BUILD_SHARED_LIBS)
    set(ROOTHELPER_CLING_LIBRARIES "")
    foreach(target Graphics Container DataSaver Analysis)
        get_target_property(output_name ${target} OUTPUT_NAME)
        string(APPEND ROOTHELPER_CLING_LIBRARIES
            "R__LOAD_LIBRARY(ROOTHELPER_LIBDIR/${CMAKE_SHARED_LIBRARY_PREFIX}${output_name}${CMAKE_SHARED_LIBRARY_SUFFIX})\n")
    endforeach()
else()
    set(ROOTHELPER_CLING_LIBRARIES
        "#error roothelper was built as static libraries; include roothelper_source.h instead\n")
endif()
install(CODE "
    set(ROOTHELPER_LIBDIR \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}\")
    set(ROOTHELPER_CLING_LIBRARIES \"${ROOTHELPER_CLING_LIBRARIES}\")
    string(REPLACE ROOTHELPER_LIBDIR \"\${ROOTHELPER_LIBDIR}\" ROOTHELPER_CLING_LIBRARIES \"\${ROOTHELPER_CLING_LIBRARIES}\")
    set(header \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/cling_libraries.h\")
    message(STATUS \"Installing: \${header}\")
    configure_file(\"${ROOTHELPER_CLING_TEMPLATE}\" \"\${header}\" @ONLY)
")
