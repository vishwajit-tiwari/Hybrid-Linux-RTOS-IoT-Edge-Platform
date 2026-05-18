/**
 * @file iot_uart_driver.c
 * @brief A simple Linux kernel module for UART communication
 * @details This module creates a character device that can be used for UART communication
 * in an IoT environment. It provides basic file operations for reading and writing
 * data, and can be extended to include actual UART communication logic.
 * @note This is a basic template and does not include actual UART communication logic.
 *       It is intended for educational purposes and can be extended as needed.
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
#include <linux/kfifo.h>
#include <linux/mutex.h>


/// Device information
#define DEVICE_NAME "iot_uart"
#define CLASS_NAME  "iot_class"

// FIFO buffer size for UART communication
#define FIFO_SIZE 1024

/// Global variables for device management
static dev_t dev_num;
static struct cdev iot_cdev;
static struct class *iot_class;

// FIFO buffer for UART communication
static DECLARE_KFIFO(iot_fifo, char, FIFO_SIZE);

// Mutex for synchronizing access to the FIFO buffer
static DEFINE_MUTEX(iot_mutex);

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
    
    char kbuf[128];
    int copied;

    mutex_lock(&iot_mutex);

    if (kfifo_is_empty(&iot_fifo)) 
    {
        mutex_unlock(&iot_mutex);
        pr_info("iot_uart: FIFO empty\n");
        return 0;
    }

    copied = kfifo_out(&iot_fifo, kbuf, min((unsigned int)len, kfifo_len(&iot_fifo)));

    mutex_unlock(&iot_mutex);

    if (copy_to_user(buf, kbuf, copied))
    {
        return -EFAULT;
    }

    pr_info("iot_uart: read %d bytes from FIFO\n", copied);

    return copied;
}

// write function - called when data is written to the device
static ssize_t iot_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    pr_info("iot_uart: write called\n");
    
    char kbuf[128];

    if (len > sizeof(kbuf))
    {
        len = sizeof(kbuf);
    }

    if (copy_from_user(kbuf, buf, len))
    {
        return -EFAULT;
    }

    mutex_lock(&iot_mutex);

    if (kfifo_avail(&iot_fifo) < len) 
    {
        mutex_unlock(&iot_mutex);

        pr_err("iot_uart: FIFO full\n");

        return -ENOSPC;
    }

    kfifo_in(&iot_fifo, kbuf, len);

    mutex_unlock(&iot_mutex);

    pr_info("iot_uart: wrote %zu bytes into FIFO\n", len);

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

    mutex_init(&iot_mutex); // Initialize the mutex for synchronizing access to the FIFO buffer

    INIT_KFIFO(iot_fifo); // Initialize the FIFO buffer

    pr_info("iot_uart: buffered driver loaded\n");

    return 0;
}

// Module exit function - called when the module is unloaded
static void __exit iot_driver_exit(void)
{
    device_destroy(iot_class, dev_num); // Remove the device node
    class_destroy(iot_class); // Destroy the device class
    cdev_del(&iot_cdev); // Remove the character device from the kernel
    unregister_chrdev_region(dev_num, 1); // Unregister the device number

    mutex_destroy(&iot_mutex); // Destroy the mutex
    kfifo_free(&iot_fifo); // Free the FIFO buffer

    pr_info("iot_uart: driver unloaded\n");
}

module_init(iot_driver_init);
module_exit(iot_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishwajit Kumar Tiwari");
MODULE_DESCRIPTION("Buffered Hybrid Linux-RTOS IoT UART Driver");