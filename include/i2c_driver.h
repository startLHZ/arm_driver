#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <linux/i2c.h>

// I2C读取数据
static inline int i2c_read_reg(struct i2c_client *client, u8 reg, u8 *val)
{
    return i2c_smbus_read_byte_data(client, reg);
}

// I2C写入数据
static inline int i2c_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
    return i2c_smbus_write_byte_data(client, reg, val);
}

#endif /* I2C_DRIVER_H */

