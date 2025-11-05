# CMakeLists.txt for Network Block Device Driver

# 创建 Kbuild 文件用于内核模块编译
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Kbuild "obj-m += net_block_driver.o\n")

# 复制源文件到构建目录
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/net_block_driver.c
    ${CMAKE_CURRENT_BINARY_DIR}/net_block_driver.c
    COPYONLY
)

# 复制 README
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ${CMAKE_CURRENT_BINARY_DIR}/README.md
        COPYONLY
    )
endif()

# 添加自定义构建步骤来编译和复制模块
add_custom_target(copy_net_block_driver ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/output
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/net_block_driver.ko
        ${CMAKE_BINARY_DIR}/output/net_block_driver.ko
    DEPENDS modules_net_block_driver
    COMMENT "Copying net_block_driver.ko to output directory"
)
