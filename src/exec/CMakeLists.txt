add_executable(TestGraphics test_graphics.cpp)
target_link_libraries(TestGraphics PRIVATE
    RootHelper
)

add_executable(TestDataSaver test_data_saver.cpp)
target_link_libraries(TestDataSaver PRIVATE
    RootHelper
)

add_executable(TestLogAxisLabels test_log_axis_labels.cpp)
target_link_libraries(TestLogAxisLabels PRIVATE RootHelper)
