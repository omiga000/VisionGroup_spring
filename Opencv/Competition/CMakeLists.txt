add_executable(armor Src/armor.cpp)

target_link_libraries(armor PRIVATE ${OpenCV_LIBS})