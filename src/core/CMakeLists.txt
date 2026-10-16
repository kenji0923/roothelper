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
)
target_sources(RootHelper PUBLIC
    FILE_SET top_header_for_interpreter
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../roothelper.h
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
