# Flash+RAM 混合块设备驱动

基于 Linux blk-mq 框架的混合块设备驱动，结合 Serial NOR Flash 和 RAM，提供可格式化、可挂载的完整块设备功能。

v27 ADCU，上电mcu初始化环视模组即可使用。

## 📋 设备架构

### 设备布局
```
[RAM 区域: 0 - 3MB]  [Flash 区域: 3MB - 3.5MB]
     可读写              可读写（带擦除）
```

| 区域 | 大小 | 偏移 |
|------|------|------|
| RAM | 3MB | 0 - 3MB |
| Flash | 512KB | 3MB - 3.5MB |

- **总大小**: 3.5MB (7168 个 512 字节扇区)
- **块设备扇区**: 512 字节
- **Flash 擦除单位**: 4KB (128 个扇区，ID 0-127)

## 🔑 核心函数说明

### 1. Flash I2C 通信层

#### `flash_i2c_read_retry()`
**功能**: I2C 读取 Flash 数据，支持自动重试  
**参数**:
- `bus_num`: I2C 总线号
- `addr`: 设备地址
- `reg_addr`: 寄存器地址
- `buf`: 数据缓冲区
**重试**: 3 次  
**返回**: 成功 0，失败负数

#### `flash_i2c_write_retry()`
**功能**: I2C 写入 Flash 数据，支持自动重试  
**关键**: 合并寄存器地址和数据一起发送  
**重试**: 3 次

#### `hb_vin_i2c_write_reg16_data8()`
**功能**: 写入 16位寄存器 + 8位数据  
**用途**: Flash 控制命令
- `0xFFFF, 0xF4` - 解锁 Flash
- `0xFFFF, 0xF7` - 设置读/写模式
- `0xFFFF, 0xF5` - 锁定 Flash

### 2. Flash 操作层

#### `flash_read_raw()`
**功能**: 从 Flash 物理地址读取数据  
**流程**:
```
1. 解锁 Flash (0xFFFF ← 0xF4)
2. 设置读模式 (0xFFFF ← 0xF7)  
3. 发送读子命令: 0x8000 + 0x01 + [addr(4字节)] + 0x5A
4. 循环读取数据 (每次最多 30 字节)
   - 寄存器: 0x00 + offset
   - 数据缓冲区最大 32 字节
5. 锁定 Flash (0xFFFF ← 0xF5)
```
**延时**: 每次读取后等待 10-11ms

#### `flash_erase_sector()`
**功能**: 擦除指定扇区 (4KB)  
**参数**: `sector_id` (0-127)  
**流程**:
```
1. 解锁 Flash
2. 设置访问模式
3. 发送擦除命令: 0x8000 + [0x03, 0x00, sector_addr, 0x00, 0x5A]
   sector_addr = (sector_id >> 4) << 16 | (sector_id & 0xF) << 12
4. 等待擦除完成 (50-52ms)
5. 锁定 Flash
```
**注意**: Flash 必须先擦除才能写入

#### `flash_write()`
**功能**: 写入数据到 Flash  
**流程**:
```
1. 解锁 Flash + 设置写模式
2. 分块写入缓冲区 (每次 16 字节)
   - 寄存器: 0x00 + flash_offset
   - 最多累积 256 字节
3. 发送写入命令: 0x8000 + [0x02, 0x00, addr(3字节), 0x5A]
4. 等待写入完成 (3-4ms)
5. 锁定 Flash
```
**限制**: 每次最多写入 256 字节  
**延时**: 每次写入后等待 3-4ms

#### `flash_write_with_erase()`
**功能**: 带自动擦除的 Flash 写入  
**流程**:
```
1. 计算受影响的扇区: start = offset/4096, end = (offset+bytes-1)/4096
2. 逐个擦除扇区 (50-52ms/扇区)
3. 调用 flash_write() 写入数据
```
**性能影响**: 写入 1 字节也会擦除整个 4KB 扇区

### 3. 混合块设备层

#### `hybrid_read()`
**功能**: 统一的读取接口  
**逻辑**:
```c
if (offset < RAM_DATA_SIZE) {
    // 从 RAM 读取 (memcpy)
    if (跨区域) {
        // 继续从 Flash 读取 (flash_read_raw)
    }
} else {
    // 从 Flash 读取 (flash_read_raw)
}
```
**性能**: RAM 读取快 (~ns级)，Flash 读取慢 (~10ms/次)

#### `hybrid_write()`
**功能**: 统一的写入接口  
**逻辑**:
```c
if (offset < RAM_DATA_SIZE) {
    // 写入 RAM (memcpy)
    if (跨区域) {
        // 调用 flash_write_with_erase()
    }
} else {
    // 调用 flash_write_with_erase()
}
```
**性能**: RAM 写入快 (~ns级)，Flash 写入慢 (~50ms/4KB扇区)

#### `myblk_request()`
**功能**: blk-mq 请求处理函数  
**调用链**: `用户I/O → VFS → Block Layer → myblk_request() → hybrid_read/write()`  
**流程**:
```
1. blk_mq_start_request(req) - 开始处理
2. 分配临时缓冲区 kmalloc(total_len)
3. 写请求: 从 bio 复制数据到缓冲区
   rq_for_each_segment(bvec) {
       kmap_atomic() → memcpy() → kunmap_atomic()
   }
4. mutex_lock(&dev->lock) - 加锁
5. 调用 hybrid_read() 或 hybrid_write()
6. mutex_unlock(&dev->lock) - 解锁
7. 读请求: 从缓冲区复制数据到 bio
8. kfree(temp_buf) - 释放缓冲区
9. blk_mq_end_request(req, ret) - 完成请求
```
**关键点**: 
- 使用 `BLK_MQ_F_BLOCKING` 标志，允许睡眠
- Mutex 保证 Flash I/O 原子性
- 使用临时缓冲区避免分散内存操作

### 4. 块设备初始化

#### `myblk_init()`
**功能**: 驱动初始化  
**步骤**:
```
1. kzalloc(myblk_device) - 分配设备结构
2. vmalloc(RAM_DATA_SIZE) - 分配 RAM 缓冲区
3. 初始化 Flash I2C 参数
   flash_info.bus_num = 4
   flash_info.sensor_addr = 0x11
4. mutex_init(&lock) - 初始化互斥锁
5. register_blkdev(0, "flashblk") - 注册块设备
6. 初始化 blk-mq tag set
   .ops = &myblk_mq_ops
   .nr_hw_queues = 1
   .queue_depth = 128
   .flags = BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_BLOCKING
7. blk_mq_alloc_disk() - 分配 gendisk
8. set_capacity(gd, size/512) - 设置容量
9. blk_queue_logical_block_size(queue, 512)
10. add_disk(gd) - 添加磁盘
```

#### `myblk_exit()`
**功能**: 驱动清理  
**顺序**: 按相反顺序释放所有资源

## 🚀 使用方法

### 编译
```bash
./build.sh
scp build/output/block_driver.ko target:/userdata/myblk/
```

### 加载驱动
```bash
insmod /userdata/myblk/block_driver.ko
dmesg | tail -10
# 应该看到:
# flashblk: Device size: 3670016 bytes (7168 sectors)
# flashblk: RAM region (read-write): 0 - 3145728 bytes
# flashblk: Flash region (read-write with erase): 3145728 - 3670016 bytes
```

### 格式化并挂载
```bash
# 格式化为 ext4
mkfs.ext4 /dev/flashblk

# 挂载
mkdir -p /mnt/flashblk
mount /dev/flashblk /mnt/flashblk

# 使用
df -h /mnt/flashblk
echo "test" > /mnt/flashblk/test.txt
cat /mnt/flashblk/test.txt

# 同步并卸载
sync
umount /mnt/flashblk
```

### 卸载驱动
```bash
rmmod block_driver
```

## ⚙️ 配置参数

修改 `block_driver.c`:

```c
// I2C 参数
#define FLASH_I2C_BUS 4        // I2C 总线号
#define FLASH_I2C_ADDR 0x11    // Flash 设备地址

// 大小参数
#define RAM_DATA_SIZE (3 * 1024 * 1024)   // 3MB RAM
#define FLASH_DATA_SIZE (512 * 1024)       // 512KB Flash
#define FLASH_SECTOR_SIZE 4096             // 4KB 擦除单位
#define FLASH_MAX_SECTORS 128              // 扇区数量 0-127
```

## ⚠️ 注意事项

1. **格式化警告**: 格式化会擦除所有数据，包括 Flash 区域
2. **扇区对齐**: Flash 写入建议按 4KB 对齐
3. **断电保护**: 只有 Flash 区域断电不丢失
4. **I2C 配置**: 确保总线和地址配置正确
5. **内核版本**: 需要支持 blk-mq (kernel 3.13+)

## 🐛 调试

### 查看驱动日志
```bash
dmesg | grep flashblk
```

### 查看详细 Flash 操作
```bash
echo 8 > /proc/sys/kernel/printk  # 开启 DEBUG 日志
dmesg -w | grep flashblk
```

### 查看设备信息
```bash
cat /proc/partitions | grep flashblk
blockdev --getsize64 /dev/flashblk
cat /sys/block/flashblk/size
```

## 🔍 故障排查

### 问题1: 挂载失败
```bash
# 检查格式化是否成功
dmesg | grep -E "flashblk|ext4"

# 确认设备大小
blockdev --getsize64 /dev/flashblk  # 应该是 3670016

# 重新格式化
umount /mnt/flashblk
mkfs.ext4 -F /dev/flashblk
```

### 问题2: Flash 写入失败
```bash
# 检查扇区 ID 范围
dmesg | grep "Invalid sector_id"

# 检查 I2C 通信
i2cdetect -y 4  # 应该能看到 0x11 设备

# 查看详细错误
dmesg | tail -50
```

### 问题3: 性能问题
- 文件系统会优先使用前 3MB 的 RAM 区域
- 避免频繁小块写入 Flash
- 大文件写入考虑增大 RAM 区域


## 📄 许可证

GPL v2

## 👤 作者

Huaizhen.lv

---

**版本**: 1.0  
**更新日期**: 2025-11-04
