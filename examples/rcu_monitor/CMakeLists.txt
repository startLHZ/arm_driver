# RCU Monitor Test Application

add_executable(rcu_stall rcu_stall.c)
target_include_directories(rcu_stall PRIVATE ../../include)

# 复制可执行文件到输出目录
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)

add_custom_command(TARGET rcu_stall POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rcu_stall> ${OUTPUT_DIR}/
    COMMENT "复制 rcu_stall 到 ${OUTPUT_DIR}")

