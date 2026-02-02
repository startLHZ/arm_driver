#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/of.h>

#define DRIVER_NAME "i2c_simple"

static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk(KERN_INFO "%s: I2C device probed at address 0x%02x\n", 
           DRIVER_NAME, client->addr);
    return 0;
}

static void i2c_remove(struct i2c_client *client)
{
    printk(KERN_INFO "%s: I2C device removed\n", DRIVER_NAME);
}

static const struct i2c_device_id i2c_id[] = {
    { "i2c_simple", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, i2c_id);

static const struct of_device_id i2c_of_match[] = {
    { .compatible = "simple,i2c-device" },
    { }
};
MODULE_DEVICE_TABLE(of, i2c_of_match);

static struct i2c_driver i2c_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = i2c_of_match,
    },
    .probe = i2c_probe,
    .remove = i2c_remove,
    .id_table = i2c_id,
};

module_i2c_driver(i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Developer");
MODULE_DESCRIPTION("Simple I2C Driver");
MODULE_VERSION("1.0");

