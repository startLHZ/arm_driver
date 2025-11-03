# Simple Driver CMakeLists.txt

# 复制Makefile为Kbuild（内核构建系统使用）和源文件到构建目录
configure_file(Makefile ${CMAKE_CURRENT_BINARY_DIR}/Kbuild COPYONLY)
configure_file(simple_driver.c ${CMAKE_CURRENT_BINARY_DIR}/simple_driver.c COPYONLY)
