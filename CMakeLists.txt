cmake_minimum_required(VERSION 3.11)

project(NeoPspFetch)

add_executable(${PROJECT_NAME} main.c kernel.c)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pspdebug
    pspdisplay
    pspge
    psppower
    psprtc
    pspumd
    pspctrl
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libpspexploit.a"
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-int-conversion -trigraphs)

# Create an EBOOT.PBP file
create_pbp_file(
    TARGET ${PROJECT_NAME}
    ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/img/ICON0.png"
    BACKGROUND_PATH "${CMAKE_CURRENT_SOURCE_DIR}/img/PIC1.png"
    PREVIEW_PATH NULL
    TITLE ${PROJECT_NAME}
    VERSION 01.00
)
