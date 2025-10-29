# Linux ARM Driver Framework

A simple Linux ARM driver framework with CMake build system.

## Structure

- `src/` - Driver source code
- `include/` - Header files  
- `examples/` - Test applications
- `build.sh` - Build script

## Build

```bash
./build.sh
```

## Load/Unload Modules

```bash
sudo insmod src/simple_driver.ko
sudo insmod src/char_driver.ko
sudo rmmod simple_driver
sudo rmmod char_driver
```

## Test

```bash
./examples/test_app
```
