add_executable(apple Src/Apple.cpp)

# ${OpenCV_LIBS} 已包含 opencv_geometry,不需要额外单列
target_link_libraries(apple PRIVATE ${OpenCV_LIBS})