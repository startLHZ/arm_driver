cmake_minimum_required(VERSION 3.16)
project(arm_driver)

set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

find_path(KERNEL_DIR
    NAMES Makefile
    PATHS /lib/modules/${CMAKE_SYSTEM_VERSION}/build
          /usr/src/linux-headers-${CMAKE_SYSTEM_VERSION}
          /usr/src/linux
    DOC "Kernel build directory"
)

if(NOT KERNEL_DIR)
    message(FATAL_ERROR "Kernel build directory not found")
endif()

set(KBUILD_CMD ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_DIR} M=${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(examples)

add_custom_target(modules
    COMMAND ${KBUILD_CMD} modules
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building kernel modules"
)

add_custom_target(modules_install
    COMMAND ${KBUILD_CMD} modules_install
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Installing kernel modules"
)

add_custom_target(clean_modules
    COMMAND ${KBUILD_CMD} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Cleaning kernel modules"
)
