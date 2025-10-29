#!/bin/bash

mkdir -p build
cd build
cmake ..
make

echo "Building kernel modules..."
make modules

echo "Build completed!"
echo "To install modules: sudo make modules_install"
echo "To load simple driver: sudo insmod src/simple_driver.ko"
echo "To load char driver: sudo insmod src/char_driver.ko"
echo "To test char driver: ./examples/test_app"
