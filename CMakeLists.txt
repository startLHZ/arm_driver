cmake_minimum_required(VERSION 3.16)
project(arm_driver C)

set(CMAKE_C_STANDARD 99)

# 编译类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译器选项
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# ARM交叉编译配置 (由build.sh传入)
set(ARCH "arm" CACHE STRING "Target architecture")
set(CROSS_COMPILE "arm-linux-gnueabihf-" CACHE STRING "Cross compiler prefix")
set(KERNEL_DIR "" CACHE PATH "Kernel build directory")

message(STATUS "=====================================")
message(STATUS "ARM驱动编译配置")
message(STATUS "=====================================")
message(STATUS "架构:         ${ARCH}")
message(STATUS "交叉编译器:   ${CROSS_COMPILE}")
message(STATUS "内核路径:     ${KERNEL_DIR}")
message(STATUS "=====================================")

add_subdirectory(src)
add_subdirectory(examples)

# 内核模块编译 - 为每个驱动单独编译
set(DRIVER_DIRS simple_driver block_driver net_block_driver char_driver)

add_custom_target(modules ALL
    COMMENT "Building all kernel modules"
)

foreach(DRIVER_DIR ${DRIVER_DIRS})
    set(DRIVER_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/${DRIVER_DIR})
    set(KBUILD_CMD ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_DIR} M=${DRIVER_BUILD_DIR} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE})
    
    add_custom_target(modules_${DRIVER_DIR}
        COMMAND ${KBUILD_CMD} modules
        WORKING_DIRECTORY ${DRIVER_BUILD_DIR}
        COMMENT "Building ${DRIVER_DIR} kernel module"
    )
    
    add_dependencies(modules modules_${DRIVER_DIR})
endforeach()

add_custom_target(clean_modules
    COMMENT "Cleaning all kernel modules"
)

foreach(DRIVER_DIR ${DRIVER_DIRS})
    set(DRIVER_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/${DRIVER_DIR})
    set(KBUILD_CMD ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_DIR} M=${DRIVER_BUILD_DIR} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE})
    
    add_custom_target(clean_modules_${DRIVER_DIR}
        COMMAND ${KBUILD_CMD} clean
        WORKING_DIRECTORY ${DRIVER_BUILD_DIR}
    )
    
    add_dependencies(clean_modules clean_modules_${DRIVER_DIR})
endforeach()
