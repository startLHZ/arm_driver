/*
 * Network Block Device Server
 * 
 * This is a simple server implementation for the network block device driver.
 * It accepts TCP connections and handles READ/WRITE requests to a backing file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdint.h>

#define SECTOR_SIZE 512
#define BUFFER_SIZE (1024 * 1024)  /* 1MB buffer */

/* Protocol commands */
#define NET_CMD_READ       0x01
#define NET_CMD_WRITE      0x02
#define NET_CMD_DISCONNECT 0x03

/* Protocol responses */
#define NET_STATUS_OK      0x00
#define NET_STATUS_ERROR   0x01

/* Request packet structure */
struct net_request_packet {
    uint8_t cmd;
    uint64_t sector;
    uint32_t length;
} __attribute__((packed));

/* Response packet structure */
struct net_response_packet {
    uint8_t status;
} __attribute__((packed));

/* Server configuration */
struct server_config {
    int port;
    char *storage_file;
    size_t storage_size;
    int fd;
    volatile int running;
};

static struct server_config config;

/* Convert big-endian to host byte order */
static uint64_t be64toh_manual(uint64_t value) {
    uint64_t result = 0;
    uint8_t *p = (uint8_t *)&value;
    result = ((uint64_t)p[0] << 56) |
             ((uint64_t)p[1] << 48) |
             ((uint64_t)p[2] << 40) |
             ((uint64_t)p[3] << 32) |
             ((uint64_t)p[4] << 24) |
             ((uint64_t)p[5] << 16) |
             ((uint64_t)p[6] << 8) |
             ((uint64_t)p[7]);
    return result;
}

static uint32_t be32toh_manual(uint32_t value) {
    uint32_t result = 0;
    uint8_t *p = (uint8_t *)&value;
    result = ((uint32_t)p[0] << 24) |
             ((uint32_t)p[1] << 16) |
             ((uint32_t)p[2] << 8) |
             ((uint32_t)p[3]);
    return result;
}

/* Send data */
static int send_all(int sock, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(sock, buf + sent, len - sent, 0);
        if (n < 0) {
            perror("send");
            return -1;
        }
        sent += n;
    }
    return 0;
}

/* Receive data */
static int recv_all(int sock, void *buf, size_t len) {
    size_t received = 0;
    while (received < len) {
        ssize_t n = recv(sock, buf + received, len - received, 0);
        if (n < 0) {
            perror("recv");
            return -1;
        }
        if (n == 0) {
            fprintf(stderr, "Connection closed by client\n");
            return -1;
        }
        received += n;
    }
    return 0;
}

/* Handle READ request */
static int handle_read(int sock, uint64_t sector, uint32_t length) {
    struct net_response_packet resp;
    void *buffer;
    off_t offset;
    ssize_t n;
    
    printf("READ: sector=%lu, length=%u\n", sector, length);
    
    /* Validate parameters */
    offset = sector * SECTOR_SIZE;
    if ((size_t)(offset + length) > config.storage_size) {
        fprintf(stderr, "Read beyond storage size\n");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        return -1;
    }
    
    /* Allocate buffer */
    buffer = malloc(length);
    if (!buffer) {
        perror("malloc");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        return -1;
    }
    
    /* Read from storage file */
    if (lseek(config.fd, offset, SEEK_SET) != offset) {
        perror("lseek");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        free(buffer);
        return -1;
    }
    
    n = read(config.fd, buffer, length);
    if (n != length) {
        perror("read");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        free(buffer);
        return -1;
    }
    
    /* Send response */
    resp.status = NET_STATUS_OK;
    if (send_all(sock, &resp, sizeof(resp)) < 0) {
        free(buffer);
        return -1;
    }
    
    /* Send data */
    if (send_all(sock, buffer, length) < 0) {
        free(buffer);
        return -1;
    }
    
    free(buffer);
    return 0;
}

/* Handle WRITE request */
static int handle_write(int sock, uint64_t sector, uint32_t length) {
    struct net_response_packet resp;
    void *buffer;
    off_t offset;
    ssize_t n;
    
    printf("WRITE: sector=%lu, length=%u\n", sector, length);
    
    /* Validate parameters */
    offset = sector * SECTOR_SIZE;
    if ((size_t)(offset + length) > config.storage_size) {
        fprintf(stderr, "Write beyond storage size\n");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        /* Still need to receive data to keep protocol in sync */
        buffer = malloc(length);
        if (buffer) {
            recv_all(sock, buffer, length);
            free(buffer);
        }
        return -1;
    }
    
    /* Allocate buffer */
    buffer = malloc(length);
    if (!buffer) {
        perror("malloc");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        return -1;
    }
    
    /* Receive data */
    if (recv_all(sock, buffer, length) < 0) {
        free(buffer);
        return -1;
    }
    
    /* Write to storage file */
    if (lseek(config.fd, offset, SEEK_SET) != offset) {
        perror("lseek");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        free(buffer);
        return -1;
    }
    
    n = write(config.fd, buffer, length);
    if (n != length) {
        perror("write");
        resp.status = NET_STATUS_ERROR;
        send_all(sock, &resp, sizeof(resp));
        free(buffer);
        return -1;
    }
    
    /* Sync to disk */
    fsync(config.fd);
    
    /* Send response */
    resp.status = NET_STATUS_OK;
    if (send_all(sock, &resp, sizeof(resp)) < 0) {
        free(buffer);
        return -1;
    }
    
    free(buffer);
    return 0;
}

/* Handle client connection */
static void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    struct net_request_packet req;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    
    getpeername(client_sock, (struct sockaddr *)&addr, &addr_len);
    printf("Client connected: %s:%d\n",
           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    
    /* Set socket options */
    int flag = 1;
    setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    
    /* Handle requests */
    while (config.running) {
        /* Receive request header */
        if (recv_all(client_sock, &req, sizeof(req)) < 0) {
            break;
        }
        
        /* Convert from network byte order */
        uint64_t sector = be64toh_manual(req.sector);
        uint32_t length = be32toh_manual(req.length);
        
        /* Handle command */
        switch (req.cmd) {
        case NET_CMD_READ:
            if (handle_read(client_sock, sector, length) < 0) {
                goto disconnect;
            }
            break;
            
        case NET_CMD_WRITE:
            if (handle_write(client_sock, sector, length) < 0) {
                goto disconnect;
            }
            break;
            
        case NET_CMD_DISCONNECT:
            printf("Disconnect requested by client\n");
            goto disconnect;
            
        default:
            fprintf(stderr, "Unknown command: 0x%02x\n", req.cmd);
            goto disconnect;
        }
    }
    
disconnect:
    printf("Client disconnected: %s:%d\n",
           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    close(client_sock);
    return NULL;
}

/* Signal handler */
static void signal_handler(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    config.running = 0;
}

/* Initialize storage file */
static int init_storage(const char *path, size_t size) {
    int fd;
    
    /* Open or create storage file */
    fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open storage file");
        return -1;
    }
    
    /* Set file size */
    if (ftruncate(fd, size) < 0) {
        perror("ftruncate");
        close(fd);
        return -1;
    }
    
    printf("Storage initialized: %s (%zu bytes)\n", path, size);
    return fd;
}

int main(int argc, char *argv[]) {
    int server_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread;
    int opt = 1;
    
    /* Parse command line */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <port> <storage_file> <size_mb>\n", argv[0]);
        fprintf(stderr, "Example: %s 10809 /tmp/netblk.img 100\n", argv[0]);
        return 1;
    }
    
    config.port = atoi(argv[1]);
    config.storage_file = argv[2];
    config.storage_size = atoi(argv[3]) * 1024 * 1024;
    config.running = 1;
    
    /* Initialize storage */
    config.fd = init_storage(config.storage_file, config.storage_size);
    if (config.fd < 0) {
        return 1;
    }
    
    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Create server socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        close(config.fd);
        return 1;
    }
    
    /* Set socket options */
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_sock);
        close(config.fd);
        return 1;
    }
    
    /* Bind */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(config.port);
    
    if (bind(server_sock, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        close(config.fd);
        return 1;
    }
    
    /* Listen */
    if (listen(server_sock, 5) < 0) {
        perror("listen");
        close(server_sock);
        close(config.fd);
        return 1;
    }
    
    printf("Network Block Device Server\n");
    printf("Listening on port %d\n", config.port);
    printf("Storage: %s (%zu MB)\n",
           config.storage_file, config.storage_size / (1024 * 1024));
    printf("Press Ctrl+C to stop\n\n");
    
    /* Accept connections */
    while (config.running) {
        client_len = sizeof(client_addr);
        client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("malloc");
            continue;
        }
        
        *client_sock = accept(server_sock, (struct sockaddr *)&client_addr,
                             &client_len);
        if (*client_sock < 0) {
            if (config.running) {
                perror("accept");
            }
            free(client_sock);
            continue;
        }
        
        /* Create thread to handle client */
        if (pthread_create(&thread, NULL, handle_client, client_sock) != 0) {
            perror("pthread_create");
            close(*client_sock);
            free(client_sock);
            continue;
        }
        
        pthread_detach(thread);
    }
    
    /* Cleanup */
    close(server_sock);
    close(config.fd);
    
    printf("Server stopped\n");
    return 0;
}
