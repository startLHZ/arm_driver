#ifndef ARM_DRIVER_H
#define ARM_DRIVER_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#define DRIVER_VERSION "1.0"
#define DRIVER_AUTHOR "Developer"

struct driver_data {
    void __iomem *base;
    int irq;
    struct device *dev;
};

#endif
