set(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/camera/*.h")

add_library(camera SHARED camera.c image.c ${HEADER_LIST})

target_include_directories(camera PUBLIC ../../include/camera)
target_link_libraries(camera v4l2)