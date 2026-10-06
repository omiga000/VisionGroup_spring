




add_executable(Base Src/main.cpp)
add_executable(find_hsv Src/find_hsv.cpp)
add_executable(Video Src/Video.cpp)
add_executable(Cat Src/Cat.cpp)

# ${OpenCV_LIBS} 是一组 imported target,链上它就自动带上了头文件搜索路径,
# 所以不需要再写 target_include_directories(... ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Base PRIVATE ${OpenCV_LIBS})
target_link_libraries(find_hsv PRIVATE ${OpenCV_LIBS})
target_link_libraries(Video PRIVATE ${OpenCV_LIBS})
target_link_libraries(Cat PRIVATE ${OpenCV_LIBS})






