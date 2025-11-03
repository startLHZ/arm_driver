#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    int fd;
    char buffer[256];
    char write_data[] = "Hello driver";
    
    fd = open("/dev/char_dev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    
    printf("Writing to device: %s\n", write_data);
    write(fd, write_data, strlen(write_data));
    
    printf("Reading from device:\n");
    read(fd, buffer, sizeof(buffer));
    printf("%s", buffer);
    
    close(fd);
    return 0;
}
