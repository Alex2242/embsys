set(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/utils/*.h")

add_library(utils SHARED utils.c led.c ${HEADER_LIST})

target_include_directories(utils PUBLIC ../../include/utils)