/*
 * test_chardev.c - 字符设备驱动测试程序
 * 
 * 功能：
 * - 测试字符设备的基本操作（open/read/write/close）
 * - 测试 lseek 功能
 * - 测试 ioctl 命令
 * - 压力测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_PATH "/dev/mychardev"

/* IOCTL 命令定义（与驱动一致） */
#define CHARDEV_IOC_MAGIC  'k'
#define CHARDEV_IOCRESET    _IO(CHARDEV_IOC_MAGIC,  0)
#define CHARDEV_IOCGETSIZE  _IOR(CHARDEV_IOC_MAGIC, 1, int)
#define CHARDEV_IOCGETUSED  _IOR(CHARDEV_IOC_MAGIC, 2, int)

/* 颜色输出宏 */
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_RESET   "\033[0m"

#define TEST_PASS(msg) printf(COLOR_GREEN "✓ PASS: %s\n" COLOR_RESET, msg)
#define TEST_FAIL(msg) printf(COLOR_RED "✗ FAIL: %s\n" COLOR_RESET, msg)
#define TEST_INFO(msg) printf(COLOR_BLUE "ℹ INFO: %s\n" COLOR_RESET, msg)
#define TEST_WARN(msg) printf(COLOR_YELLOW "⚠ WARN: %s\n" COLOR_RESET, msg)

/* 测试结果统计 */
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

void test_result(int passed, const char *test_name) {
    total_tests++;
    if (passed) {
        passed_tests++;
        TEST_PASS(test_name);
    } else {
        failed_tests++;
        TEST_FAIL(test_name);
    }
}

/* 1. 测试打开和关闭设备 */
int test_open_close(void) {
    printf("\n" COLOR_YELLOW "=== 测试 1: 打开/关闭设备 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        test_result(0, "打开设备");
        return -1;
    }
    test_result(1, "打开设备");
    
    int ret = close(fd);
    test_result(ret == 0, "关闭设备");
    
    return 0;
}

/* 2. 测试写入数据 */
int test_write(void) {
    printf("\n" COLOR_YELLOW "=== 测试 2: 写入数据 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    
    const char *test_data = "Hello, Character Device Driver!";
    ssize_t bytes = write(fd, test_data, strlen(test_data));
    
    test_result(bytes == strlen(test_data), "写入数据");
    printf("  写入了 %zd 字节: '%s'\n", bytes, test_data);
    
    close(fd);
    return 0;
}

/* 3. 测试读取数据 */
int test_read(void) {
    printf("\n" COLOR_YELLOW "=== 测试 3: 读取数据 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    
    /* 先写入数据 */
    const char *write_data = "Test Read Operation";
    write(fd, write_data, strlen(write_data));
    
    /* 重置文件位置 */
    lseek(fd, 0, SEEK_SET);
    
    /* 读取数据 */
    char read_buffer[256] = {0};
    ssize_t bytes = read(fd, read_buffer, sizeof(read_buffer));
    
    int match = (bytes == strlen(write_data) && 
                 strcmp(read_buffer, write_data) == 0);
    test_result(match, "读取数据并验证");
    printf("  读取了 %zd 字节: '%s'\n", bytes, read_buffer);
    
    close(fd);
    return 0;
}

/* 4. 测试 lseek */
int test_lseek(void) {
    printf("\n" COLOR_YELLOW "=== 测试 4: lseek 定位 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    
    /* 先清空设备 */
    ioctl(fd, CHARDEV_IOCRESET);
    
    /* 写入测试数据 */
    const char *data = "0123456789ABCDEF";
    write(fd, data, strlen(data));
    
    /* 测试 SEEK_SET */
    off_t pos = lseek(fd, 5, SEEK_SET);
    test_result(pos == 5, "SEEK_SET 定位到位置 5");
    
    char buf[10] = {0};
    read(fd, buf, 5);
    test_result(strcmp(buf, "56789") == 0, "从位置 5 读取正确数据");
    printf("  读取到: '%s'\n", buf);
    
    /* 测试 SEEK_CUR */
    lseek(fd, 0, SEEK_SET);  // 重置
    lseek(fd, 5, SEEK_CUR);
    pos = lseek(fd, 0, SEEK_CUR);  // 获取当前位置
    test_result(pos == 5, "SEEK_CUR 相对定位");
    
    /* 测试 SEEK_END */
    pos = lseek(fd, 0, SEEK_END);
    test_result(pos == strlen(data), "SEEK_END 定位到末尾");
    printf("  文件末尾位置: %ld\n", pos);
    
    close(fd);
    return 0;
}

/* 5. 测试 IOCTL 命令 */
int test_ioctl(void) {
    printf("\n" COLOR_YELLOW "=== 测试 5: IOCTL 命令 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    
    /* 测试获取缓冲区大小 */
    int buffer_size = 0;
    int ret = ioctl(fd, CHARDEV_IOCGETSIZE, &buffer_size);
    test_result(ret == 0 && buffer_size > 0, "获取缓冲区大小");
    printf("  缓冲区大小: %d 字节\n", buffer_size);
    
    /* 写入一些数据 */
    const char *data = "IOCTL Test Data";
    write(fd, data, strlen(data));
    
    /* 测试获取已使用大小 */
    int used_size = 0;
    ret = ioctl(fd, CHARDEV_IOCGETUSED, &used_size);
    test_result(ret == 0 && used_size == strlen(data), "获取已使用大小");
    printf("  已使用大小: %d 字节\n", used_size);
    
    /* 测试重置缓冲区 */
    ret = ioctl(fd, CHARDEV_IOCRESET);
    test_result(ret == 0, "重置缓冲区");
    
    /* 验证重置后的大小 */
    ioctl(fd, CHARDEV_IOCGETUSED, &used_size);
    test_result(used_size == 0, "重置后验证");
    printf("  重置后大小: %d 字节\n", used_size);
    
    close(fd);
    return 0;
}

/* 6. 边界测试 */
int test_boundary(void) {
    printf("\n" COLOR_YELLOW "=== 测试 6: 边界条件 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    
    /* 清空设备 */
    ioctl(fd, CHARDEV_IOCRESET);
    
    /* 获取缓冲区大小 */
    int buffer_size = 0;
    ioctl(fd, CHARDEV_IOCGETSIZE, &buffer_size);
    
    /* 测试读取空设备 */
    char buf[100];
    ssize_t bytes = read(fd, buf, sizeof(buf));
    test_result(bytes == 0, "读取空设备返回 0");
    
    /* 测试填满缓冲区 */
    char *large_buf = malloc(buffer_size);
    memset(large_buf, 'A', buffer_size);
    bytes = write(fd, large_buf, buffer_size);
    test_result(bytes == buffer_size, "填满缓冲区");
    printf("  写入了 %zd/%d 字节\n", bytes, buffer_size);
    
    /* 测试溢出 */
    bytes = write(fd, "overflow", 8);
    test_result(bytes == 0, "缓冲区满后写入返回 0");
    
    free(large_buf);
    close(fd);
    return 0;
}

/* 7. 压力测试 */
int test_stress(void) {
    printf("\n" COLOR_YELLOW "=== 测试 7: 压力测试 ===" COLOR_RESET "\n");
    
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return -1;
    }
    
    ioctl(fd, CHARDEV_IOCRESET);
    
    /* 多次小量读写 */
    int iterations = 100;
    int success = 1;
    
    for (int i = 0; i < iterations; i++) {
        char data[32];
        snprintf(data, sizeof(data), "Data-%d", i);
        
        lseek(fd, 0, SEEK_SET);
        ssize_t written = write(fd, data, strlen(data));
        
        lseek(fd, 0, SEEK_SET);
        char read_buf[32] = {0};
        ssize_t bytes_read = read(fd, read_buf, written);
        
        if (bytes_read != written || strcmp(data, read_buf) != 0) {
            success = 0;
            break;
        }
    }
    
    test_result(success, "多次读写测试");
    printf("  完成 %d 次读写操作\n", iterations);
    
    close(fd);
    return 0;
}

/* 8. 并发测试（多进程） */
int test_concurrent(void) {
    printf("\n" COLOR_YELLOW "=== 测试 8: 并发访问 ===" COLOR_RESET "\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        /* 子进程 */
        int fd = open(DEVICE_PATH, O_RDWR);
        if (fd >= 0) {
            const char *data = "Child Process Data";
            write(fd, data, strlen(data));
            close(fd);
        }
        exit(0);
    } else if (pid > 0) {
        /* 父进程 */
        int fd = open(DEVICE_PATH, O_RDWR);
        if (fd >= 0) {
            const char *data = "Parent Process Data";
            write(fd, data, strlen(data));
            close(fd);
        }
        
        /* 等待子进程 */
        wait(NULL);
        test_result(1, "多进程并发访问");
    } else {
        test_result(0, "创建子进程失败");
    }
    
    return 0;
}

/* 显示使用帮助 */
void print_usage(const char *prog_name) {
    printf("使用方法: %s [选项]\n", prog_name);
    printf("选项:\n");
    printf("  -a, --all       运行所有测试（默认）\n");
    printf("  -i, --info      显示设备信息\n");
    printf("  -w TEXT         写入自定义文本\n");
    printf("  -r              读取设备内容\n");
    printf("  -c              清空设备\n");
    printf("  -h, --help      显示此帮助信息\n");
}

/* 显示设备信息 */
void show_device_info(void) {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return;
    }
    
    int buffer_size = 0, used_size = 0;
    ioctl(fd, CHARDEV_IOCGETSIZE, &buffer_size);
    ioctl(fd, CHARDEV_IOCGETUSED, &used_size);
    
    printf("\n" COLOR_BLUE "=== 设备信息 ===" COLOR_RESET "\n");
    printf("设备路径: %s\n", DEVICE_PATH);
    printf("缓冲区大小: %d 字节\n", buffer_size);
    printf("已使用: %d 字节\n", used_size);
    printf("可用: %d 字节\n", buffer_size - used_size);
    printf("使用率: %.1f%%\n", (used_size * 100.0) / buffer_size);
    
    close(fd);
}

int main(int argc, char *argv[]) {
    printf(COLOR_BLUE "========================================\n");
    printf("   字符设备驱动测试程序 v1.0\n");
    printf("========================================\n" COLOR_RESET);
    
    /* 检查设备是否存在 */
    if (access(DEVICE_PATH, F_OK) != 0) {
        printf(COLOR_RED "错误: 设备 %s 不存在\n" COLOR_RESET, DEVICE_PATH);
        printf("请先加载驱动模块: sudo insmod char_driver.ko\n");
        return 1;
    }
    
    /* 解析命令行参数 */
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--info") == 0) {
            show_device_info();
            return 0;
        } else if (strcmp(argv[1], "-w") == 0 && argc > 2) {
            int fd = open(DEVICE_PATH, O_RDWR);
            if (fd >= 0) {
                write(fd, argv[2], strlen(argv[2]));
                printf("已写入: %s\n", argv[2]);
                close(fd);
            }
            return 0;
        } else if (strcmp(argv[1], "-r") == 0) {
            int fd = open(DEVICE_PATH, O_RDWR);
            if (fd >= 0) {
                char buf[4096] = {0};
                lseek(fd, 0, SEEK_SET);
                ssize_t bytes = read(fd, buf, sizeof(buf));
                printf("读取到 %zd 字节:\n%s\n", bytes, buf);
                close(fd);
            }
            return 0;
        } else if (strcmp(argv[1], "-c") == 0) {
            int fd = open(DEVICE_PATH, O_RDWR);
            if (fd >= 0) {
                ioctl(fd, CHARDEV_IOCRESET);
                printf("设备已清空\n");
                close(fd);
            }
            return 0;
        }
    }
    
    /* 运行所有测试 */
    test_open_close();
    test_write();
    test_read();
    test_lseek();
    test_ioctl();
    test_boundary();
    test_stress();
    test_concurrent();
    
    /* 显示测试结果统计 */
    printf("\n" COLOR_BLUE "========================================\n");
    printf("   测试结果统计\n");
    printf("========================================\n" COLOR_RESET);
    printf("总测试数: %d\n", total_tests);
    printf(COLOR_GREEN "通过: %d\n" COLOR_RESET, passed_tests);
    printf(COLOR_RED "失败: %d\n" COLOR_RESET, failed_tests);
    printf("成功率: %.1f%%\n", (passed_tests * 100.0) / total_tests);
    
    if (failed_tests == 0) {
        printf(COLOR_GREEN "\n🎉 所有测试通过！\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "\n⚠️  部分测试失败，请检查！\n" COLOR_RESET);
    }
    
    return (failed_tests == 0) ? 0 : 1;
}

