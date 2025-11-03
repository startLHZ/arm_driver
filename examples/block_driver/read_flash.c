/*
 * Flash Reader Program
 * 
 * This program reads data from flash memory at specified addresses
 * using the sysfs interface provided by the block_driver module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define SYSFS_FLASH_DATA "/sys/class/flashblk/flashblk/flash_data"
#define SYSFS_FLASH_ADDR "/sys/class/flashblk/flashblk/flash_addr"
#define SYSFS_FLASH_LEN "/sys/class/flashblk/flashblk/flash_len"

void print_usage(const char *prog) {
    printf("Flash Reader - Read data from flash memory via sysfs\n");
    printf("\nUsage: %s <address> <length>\n", prog);
    printf("\nArguments:\n");
    printf("  address  - Flash address to read (hex: 0xXXXXXX or decimal)\n");
    printf("  length   - Number of bytes to read (1-256)\n");
    printf("\nExamples:\n");
    printf("  %s 0x0C0003 16    # Read 16 bytes from 0x0C0003\n", prog);
    printf("  %s 0x000000 256   # Read 256 bytes from 0x000000\n", prog);
    printf("  %s 786432 64      # Read 64 bytes from decimal address\n", prog);
    printf("\nNote: The driver module must be loaded first:\n");
    printf("  sudo insmod block_driver.ko\n");
}

int check_sysfs() {
    if (access(SYSFS_FLASH_DATA, R_OK | W_OK) != 0) {
        fprintf(stderr, "Error: Cannot access %s\n", SYSFS_FLASH_DATA);
        fprintf(stderr, "Make sure the block_driver module is loaded:\n");
        fprintf(stderr, "  sudo insmod block_driver.ko\n");
        return -1;
    }
    return 0;
}

int write_sysfs(const char *path, const char *data) {
    int fd;
    ssize_t ret;
    
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }
    
    ret = write(fd, data, strlen(data));
    close(fd);
    
    if (ret < 0) {
        fprintf(stderr, "Error: Failed to write to %s: %s\n", path, strerror(errno));
        return -1;
    }
    
    return 0;
}

int read_flash_data(unsigned int addr, unsigned int len) {
    char cmd[128];
    FILE *fp;
    char buffer[4096];
    
    if (len < 1 || len > 256) {
        fprintf(stderr, "Error: Length must be between 1 and 256 bytes\n");
        return -1;
    }
    
    // Method 1: Use flash_data file with "address length" format
    snprintf(cmd, sizeof(cmd), "%u %u", addr, len);
    
    printf("Reading %u bytes from flash address 0x%06X...\n", len, addr);
    
    if (write_sysfs(SYSFS_FLASH_DATA, cmd) != 0) {
        return -1;
    }
    
    // Small delay to allow driver to process
    usleep(100000); // 100ms
    
    // Read the result
    fp = fopen(SYSFS_FLASH_DATA, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open %s for reading: %s\n", 
                SYSFS_FLASH_DATA, strerror(errno));
        return -1;
    }
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    
    fclose(fp);
    return 0;
}

int read_flash_data_separate(unsigned int addr, unsigned int len) {
    char cmd[128];
    FILE *fp;
    char buffer[4096];
    
    if (len < 1 || len > 256) {
        fprintf(stderr, "Error: Length must be between 1 and 256 bytes\n");
        return -1;
    }
    
    printf("Reading %u bytes from flash address 0x%06X...\n", len, addr);
    
    // Method 2: Set address and length separately
    snprintf(cmd, sizeof(cmd), "0x%X", addr);
    if (write_sysfs(SYSFS_FLASH_ADDR, cmd) != 0) {
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), "%u", len);
    if (write_sysfs(SYSFS_FLASH_LEN, cmd) != 0) {
        return -1;
    }
    
    // Trigger read by reading flash_data
    usleep(100000); // 100ms
    
    fp = fopen(SYSFS_FLASH_DATA, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open %s for reading: %s\n", 
                SYSFS_FLASH_DATA, strerror(errno));
        return -1;
    }
    
    printf("\n--- Flash Data ---\n");
    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    printf("------------------\n");
    
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    unsigned int address;
    unsigned int length;
    char *endptr;
    
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse address (supports both hex and decimal)
    errno = 0;
    address = strtoul(argv[1], &endptr, 0);
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid address '%s'\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse length
    errno = 0;
    length = strtoul(argv[2], &endptr, 0);
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid length '%s'\n", argv[2]);
        print_usage(argv[0]);
        return 1;
    }
    
    // Check if sysfs interface is available
    if (check_sysfs() != 0) {
        return 1;
    }
    
    // Read flash data
    return read_flash_data(address, length);
}
