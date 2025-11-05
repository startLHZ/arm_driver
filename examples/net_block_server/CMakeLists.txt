cmake_minimum_required(VERSION 3.16)

# 编译服务端程序
add_executable(netblk_server netblk_server.c)
target_link_libraries(netblk_server pthread)

# 安装到 output 目录
install(TARGETS netblk_server
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/output
)

# 添加到 ALL 目标
add_custom_command(TARGET netblk_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/output
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:netblk_server> ${CMAKE_BINARY_DIR}/output/
    COMMENT "Copying netblk_server to output directory"
)
