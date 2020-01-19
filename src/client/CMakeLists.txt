add_executable(client main.c)

target_include_directories(client PUBLIC ../../include/network)
target_include_directories(client PUBLIC ../../include/camera)

target_link_libraries(client camera network)