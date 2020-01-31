add_executable(server main.c daemon.c)

target_include_directories(server PUBLIC ../../include/network)
target_include_directories(server PUBLIC ../../include/camera)

target_link_libraries(server camera network utils)