set(APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/demo_main.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/pingpong_demo.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sample_echo_demo.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/console_demo.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rsc_table.c)

set(APP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_include_directories(app PRIVATE ${APP_INCLUDE_DIR})
target_sources(app PRIVATE ${APP_SRC})

# For the moment we need to include the rpmsg_lite include directory
target_include_directories(rpmsg_lite PRIVATE ${APP_INCLUDE_DIR})
