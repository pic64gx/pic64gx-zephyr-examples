cmake_minimum_required(VERSION 3.20.0)
# Copyright (c) 2020 STMicroelectronics
#
# SPDX-License-Identifier: Apache-2.0
#

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(amp_example)

# METAL_MAX_DEVICE_REGIONS is used to give the number of memory regions shared
# between processors. By default only one region is defined for the vrings
# and rpmsg buffers. The METAL_MAX_DEVICE_REGIONS has to be redefined to add a
# second region for the resource table.
zephyr_compile_definitions(METAL_MAX_DEVICE_REGIONS=2)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/application)

# MIV IHC
set(MIV_IHC_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/src/miv_ihc/miv_ihc.c)
set(MIV_IHC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/miv_ihc)

# remoteproc
set(REMOTEPROC_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/src/remoteproc/remoteproc.c)
set(REMOTEPROC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/remoteproc)

add_definitions(-DMPFS_HAL_FIRST_HART=4 -DMPFS_HAL_LAST_HART=4 -DREMOTE=1 -DREMOTEPROC=1 -DLIBERO_SETTING_CONTEXT_A_HART_EN=0x0000000EUL -DLIBERO_SETTING_CONTEXT_B_HART_EN=0x00000010UL)

zephyr_linker_sources(SECTIONS ${CMAKE_CURRENT_SOURCE_DIR}/boards/linker_overlay.ld)

target_include_directories(app PRIVATE ${PLATFORM_DIR} ${MIV_IHC_INCLUDE_DIR} ${REMOTEPROC_INCLUDE_DIR})

target_sources(app PRIVATE ${MIV_IHC_SRC} ${REMOTEPROC_SRC})

# For the moment we need to include the rpmsg_lite include directory
target_include_directories(rpmsg_lite PRIVATE ${MIV_IHC_INCLUDE_DIR} ${REMOTEPROC_INCLUDE_DIR})
target_compile_definitions(rpmsg_lite PRIVATE -DREMOTEPROC=1)
