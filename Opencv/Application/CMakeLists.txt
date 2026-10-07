add_executable(apple Src/Apple.cpp)
add_executable(calib Src/Calib.cpp)

# ${OpenCV_LIBS} 已包含 opencv_geometry,不需要额外单列
target_link_libraries(apple PRIVATE ${OpenCV_LIBS})
target_link_libraries(calib PRIVATE ${OpenCV_LIBS})