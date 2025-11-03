# 复制内核模块Makefile到特定文件名，避免被CMake覆盖
configure_file(Makefile ${CMAKE_CURRENT_BINARY_DIR}/Kbuild COPYONLY)
configure_file(simple_driver.c ${CMAKE_CURRENT_BINARY_DIR}/simple_driver.c COPYONLY)

# 复制所有其他.c文件（如果有）
file(GLOB DRIVER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
foreach(src_file ${DRIVER_SOURCES})
    get_filename_component(filename ${src_file} NAME)
    if(NOT filename STREQUAL "simple_driver.c")
        configure_file(${src_file} ${CMAKE_CURRENT_BINARY_DIR}/${filename} COPYONLY)
    endif()
endforeach()

