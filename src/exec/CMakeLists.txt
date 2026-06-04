add_executable(TestGraphics test_graphics.cpp)
target_link_libraries(TestGraphics PRIVATE
    ROOThelper
)

add_executable(TestDataSaver test_data_saver.cpp)
target_link_libraries(TestDataSaver PRIVATE
    ROOThelper
)
