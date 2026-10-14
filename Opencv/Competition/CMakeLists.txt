add_executable(armor Src/Armor.cpp)

target_link_libraries(armor PRIVATE ${OpenCV_LIBS})

add_executable(armor_red Src/Armor_red.cpp)

target_link_libraries(armor_red PRIVATE ${OpenCV_LIBS})



add_executable(videoArmor Src/VideoArmor.cpp)

target_link_libraries(videoArmor PRIVATE ${OpenCV_LIBS})


add_executable(emergy Src/Emergy.cpp)

target_link_libraries(emergy PRIVATE ${OpenCV_LIBS})
