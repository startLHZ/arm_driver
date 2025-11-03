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

# 验证内核路径
if(NOT KERNEL_DIR OR NOT EXISTS "${KERNEL_DIR}/Makefile")
    message(FATAL_ERROR 
        "无效的内核路径: ${KERNEL_DIR}\n"
        "请使用 build.sh 脚本进行编译，它会自动处理路径配置"
    )
endif()

message(STATUS "=====================================")
message(STATUS "ARM驱动编译配置")
message(STATUS "=====================================")
message(STATUS "架构:         ${ARCH}")
message(STATUS "交叉编译器:   ${CROSS_COMPILE}")
message(STATUS "内核路径:     ${KERNEL_DIR}")
message(STATUS "=====================================")

add_subdirectory(src)
add_subdirectory(examples)

# 内核模块编译命令 - 在src子目录中执行
set(KBUILD_CMD ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_DIR} M=${CMAKE_CURRENT_BINARY_DIR}/src ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE})

add_custom_target(modules
    COMMAND ${KBUILD_CMD} modules
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src
    COMMENT "Building kernel modules"
    DEPENDS src
)

add_custom_target(modules_install
    COMMAND ${KBUILD_CMD} modules_install
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src
    COMMENT "Installing kernel modules"
)

add_custom_target(clean_modules
    COMMAND ${KBUILD_CMD} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src
    COMMENT "Cleaning kernel modules"
)
