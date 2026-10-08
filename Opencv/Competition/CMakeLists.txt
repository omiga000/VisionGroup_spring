add_executable(armor Src/armor.cpp)

target_link_libraries(armor PRIVATE ${OpenCV_LIBS})

add_executable(videoArmor Src/VideoArmor.cpp)

target_link_libraries(videoArmor PRIVATE ${OpenCV_LIBS})