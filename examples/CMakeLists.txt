# 构建各个测试程序
# 每个测试程序在对应驱动的子目录中

# Simple Driver 测试程序
add_subdirectory(simple_driver)

# Block Driver 测试程序
add_subdirectory(block_driver)

# Network Block Device Server
add_subdirectory(net_block_server)

# Character Device Driver 测试程序
add_subdirectory(char_driver)
