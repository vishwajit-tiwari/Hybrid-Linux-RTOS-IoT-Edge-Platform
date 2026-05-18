/**
 * @file iot_uart_driver.c - A simple Linux kernel module for UART communication
 * 
 * @brief Hybrid Linux-RTOS IoT UART Driver
 * 
 * @details This module creates a character device that can be used for UART communication
 * in an IoT environment. It provides basic file operations for reading and writing
 * data, and can be extended to include actual UART communication logic.
 * 
 * @note This is a basic template and does not include actual UART communication logic.
 *       It is intended for educational purposes and can be extended as needed.
 * 
 * @author Vishwajit
 * @date 2026-05-18
 * @version 1.0
 * @copyright 2026 Vishwajit
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>


/// Device information
#define DEVICE_NAME "iot_uart"
#define CLASS_NAME  "iot_class"

/// Global variables for device management
static dev_t dev_num;
static struct cdev iot_cdev;
static struct class *iot_class;

// File operations prototypes
static int iot_open(struct inode *inode, struct file *file);
static int iot_release(struct inode *inode, struct file *file);
static ssize_t iot_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
static ssize_t iot_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);

// File operations structure
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = iot_open,
    .release = iot_release,
    .read    = iot_read,
    .write   = iot_write,
};

// open function - called when the device is opened
static int iot_open(struct inode *inode, struct file *file)
{
    pr_info("iot_uart: device opened\n");
    return 0;
}

// release function - called when the device is closed
static int iot_release(struct inode *inode, struct file *file)
{
    pr_info("iot_uart: device closed\n");
    return 0;
}

// read function - called when data is read from the device
static ssize_t iot_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    pr_info("iot_uart: read called\n");
    return 0;
}

// write function - called when data is written to the device
static ssize_t iot_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    pr_info("iot_uart: write called\n");
    return len;
}

// Module initialization function - called when the module is loaded
static int __init iot_driver_init(void)
{
    int ret;

    pr_info("iot_uart: initializing driver\n");

    // Allocate device number - dynamically allocate a major number
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) 
    {
        pr_err("iot_uart: alloc_chrdev_region failed\n");
        return ret;
    }

    // Log the allocated major and minor numbers
    pr_info("iot_uart: major=%d minor=%d\n", MAJOR(dev_num), MINOR(dev_num));

    // Initialize cdev - set up the character device structure
    cdev_init(&iot_cdev, &fops);

    // Add cdev to kernel - register the character device with the kernel
    ret = cdev_add(&iot_cdev, dev_num, 1);
    if (ret < 0) 
    {
        pr_err("iot_uart: cdev_add failed\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // Create device class - create a device class for the driver
    iot_class = class_create(CLASS_NAME);
    if (IS_ERR(iot_class)) 
    {
        pr_err("iot_uart: class_create failed\n");

        cdev_del(&iot_cdev); // Clean up cdev before returning
        unregister_chrdev_region(dev_num, 1); // Clean up device number before returning

        return PTR_ERR(iot_class); // Return the error code from class_create
    }

    // Create device node - create a device node in /dev
    if (IS_ERR(device_create(iot_class, NULL, dev_num, NULL, DEVICE_NAME))) 
    {
        pr_err("iot_uart: device_create failed\n");

        class_destroy(iot_class); // Clean up class before returning
        cdev_del(&iot_cdev); // Clean up cdev before returning
        unregister_chrdev_region(dev_num, 1); // Clean up device number before returning

        return -1;
    }

    pr_info("iot_uart: driver loaded successfully\n");

    return 0;
}

// Module exit function - called when the module is unloaded
static void __exit iot_driver_exit(void)
{
    device_destroy(iot_class, dev_num); // Remove the device node
    class_destroy(iot_class); // Destroy the device class
    cdev_del(&iot_cdev); // Remove the character device from the kernel
    unregister_chrdev_region(dev_num, 1); // Unregister the device number

    pr_info("iot_uart: driver unloaded\n");
}

module_init(iot_driver_init);
module_exit(iot_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishwajit Kumar Tiwari");
MODULE_DESCRIPTION("Hybrid Linux-RTOS IoT UART Driver");