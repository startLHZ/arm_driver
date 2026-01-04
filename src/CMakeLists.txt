# 构建各个驱动模块
# 每个驱动在自己的子目录中

# Simple Driver
add_subdirectory(simple_driver)

# Block Driver
add_subdirectory(block_driver)

# Network Block Driver
add_subdirectory(net_block_driver)

# Character Device Driver
add_subdirectory(char_driver)
