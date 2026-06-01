add_library(DataSaver data_saver.cpp)
target_include_directories(DataSaver PUBLIC include)
target_link_libraries(DataSaver PUBLIC
    ROOT::Gpad
)


add_library(Graphics graphics.cpp)
target_include_directories(Graphics PUBLIC include)
target_link_libraries(Graphics PUBLIC
    ROOT::Gpad
)


add_library(Container container.cpp)
target_include_directories(Container PUBLIC include)
target_link_libraries(Container PUBLIC
    ROOT::Gpad
    Graphics
)


add_library(Analysis analysis.cpp)
target_include_directories(Analysis PUBLIC include)
target_link_libraries(Analysis PUBLIC
    ROOT::Hist
)


add_library(ROOThelper INTERFACE)
target_include_directories(ROOThelper INTERFACE include)
target_link_libraries(ROOThelper INTERFACE
    DataSaver
    Graphics
    Container
    Analysis
)
target_sources(ROOThelper PUBLIC
    FILE_SET header
    TYPE HEADERS
    BASE_DIRS include/roothelper
    FILES
	include/roothelper/data_saver.h
	include/roothelper/graphics.h
	include/roothelper/container.h
	include/roothelper/analysis.h
)
target_sources(ROOThelper PUBLIC
    FILE_SET top_header_for_interpreter
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../roothelper.h
)
target_sources(ROOThelper PUBLIC
    FILE_SET source_for_interpreter
    TYPE HEADERS
    FILES
	data_saver.cpp
	graphics.cpp
	container.cpp
	analysis.cpp
)


include(GNUInstallDirs)

install(
    TARGETS
	DataSaver
	Graphics
	Container
	Analysis
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
)

install(
    TARGETS ROOThelper
    FILE_SET header
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
    TARGETS ROOThelper
    FILE_SET top_header_for_interpreter
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
    TARGETS ROOThelper
    FILE_SET source_for_interpreter
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/src
)
