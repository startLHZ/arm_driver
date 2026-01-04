# CMake configuration for char_driver test program

# 添加测试程序
add_executable(test_chardev test_chardev.c)

# 包含公共头文件目录
target_include_directories(test_chardev PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 安装规则：复制到 output 目录
install(TARGETS test_chardev DESTINATION ${CMAKE_INSTALL_PREFIX})

# 安装测试脚本
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/test_char_device.sh
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

