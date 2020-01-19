
set(HEADER_LIST "../../include/network/network.h")

add_library(network SHARED network.c ${HEADER_LIST})

target_include_directories(network PUBLIC ../../include/network)