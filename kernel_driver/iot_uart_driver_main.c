/**
 * @file iot_uart_driver.c
 * @brief Hybrid Linux-RTOS IoT UART Character Driver
 *
 * @details
 * Linux kernel character driver for a Hybrid Linux–RTOS IoT Edge Platform.
 *
 * This driver provides:
 * - character device interface
 * - UART telemetry buffering
 * - kfifo-based data management
 * - blocking and non-blocking I/O
 * - wait queue synchronization
 * - poll/select asynchronous notification
 * - telemetry frame processing framework
 * - configurable integrity validation support
 *
 * The driver is designed as a telemetry middleware layer between:
 * - ESP32 FreeRTOS telemetry producer
 * - Linux user-space telemetry services
 *
 * Intended architecture:
 *
 * ESP32 (FreeRTOS)
 *      ↓ UART Framed Telemetry
 * Linux UART Driver
 *      ↓
 * Frame Synchronization
 *      ↓
 * CRC / Checksum Validation
 *      ↓
 * kfifo Validated Buffer
 *      ↓
 * User-space Telemetry Service
 *      ↓
 * MQTT Cloud Publisher
 *
 * Features implemented:
 * - dynamic character device registration
 * - mutex-protected kfifo buffering
 * - blocking/non-blocking reads
 * - poll/select support
 * - wait queue synchronization
 * - UART integration framework
 * - asynchronous kernel thread framework
 *
 * @note
 * The current implementation focuses on stable telemetry transport,
 * modular parser integration, and Linux-side buffering architecture
 * for embedded IoT edge systems.
 *
 * @author Vishwajit Kumar Tiwari
 * @date 2026-05-22
 * @version 2.0
 *
 * @copyright
 * Copyright (c) 2026 Vishwajit Kumar Tiwari
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include <linux/kfifo.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/kthread.h>

#include "crc/integrity.h"
#include "parser/frame_parser.h"

/// Device information
#define DEVICE_NAME "iot_uart"
#define CLASS_NAME "iot_class"

// FIFO buffer size for UART communication
#define FIFO_SIZE 1024

// UART device file path (this can be adjusted based on the actual hardware
// setup)
#define UART_DEVICE "/dev/ttyAMA0"
#define UART_BUFFER_SIZE 128
#define STREAM_BUFFER_SIZE 512

/// Global variables for device management
static dev_t dev_num;
static struct cdev iot_cdev;
static struct class *iot_class;

// Thread and file pointer for UART communication
static struct task_struct *uart_thread;
static struct file *uart_filp;

// Function prototype for the UART receive thread
static int uart_rx_thread(void *data);

// FIFO buffer for UART communication
static DECLARE_KFIFO(iot_fifo, char, FIFO_SIZE);

// Mutex for synchronizing access to the FIFO buffer
static DEFINE_MUTEX(iot_mutex);

// Wait queue for handling asynchronous I/O
static DECLARE_WAIT_QUEUE_HEAD(iot_wait_queue);

// Poll function prototype for handling asynchronous I/O
static __poll_t iot_poll(struct file *file, poll_table *wait);

// File operations prototypes
static int iot_open(struct inode *inode, struct file *file);
static int iot_release(struct inode *inode, struct file *file);
static ssize_t iot_read(struct file *file, char __user *buf, size_t len,
                        loff_t *offset);
static ssize_t iot_write(struct file *file, const char __user *buf, size_t len,
                         loff_t *offset);

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = iot_open,
    .release = iot_release,
    .read = iot_read,
    .write = iot_write,
    .poll = iot_poll,
};

// open function - called when the device is opened
static int iot_open(struct inode *inode, struct file *file) {
  pr_info("iot_uart: device opened\n");
  return 0;
}

// release function - called when the device is closed
static int iot_release(struct inode *inode, struct file *file) {
  pr_info("iot_uart: device closed\n");
  return 0;
}

// read function - called when data is read from the device
static ssize_t iot_read(struct file *file, char __user *buf, size_t len,
                        loff_t *offset)                         
{
   char kbuf[FRAME_MAX_SIZE];
   unsigned int copied;
   unsigned int bytes_to_read;

   pr_info("iot_uart: read called\n");

  // Non-blocking mode support
  if (file->f_flags & O_NONBLOCK) {
    if (kfifo_is_empty(&iot_fifo)) {
      pr_info("iot_uart: FIFO empty, non-blocking read\n");
      return -EAGAIN;
    }
  } else {
    // Blocking mode - wait until data is available
    wait_event_interruptible(iot_wait_queue, !kfifo_is_empty(&iot_fifo));
  }

  mutex_lock(&iot_mutex);

  bytes_to_read =
        min_t(unsigned int,
              sizeof(kbuf),
              min_t(unsigned int,
                    len,
                    kfifo_len(&iot_fifo)));

  copied = kfifo_out(&iot_fifo, kbuf, bytes_to_read);

  mutex_unlock(&iot_mutex);

    if (copied == 0)
    {
        return 0;
    }

  if (copy_to_user(buf, kbuf, copied)) {
    return -EFAULT;
  }

  pr_info("iot_uart: read %d bytes from FIFO\n", copied);

  return copied;
}

// write function - called when data is written to the device
static ssize_t iot_write(struct file *file, const char __user *buf, size_t len,
                         loff_t *offset) {
  pr_info("iot_uart: write called\n");

  char kbuf[FRAME_MAX_SIZE];

  if (len > sizeof(kbuf)) {
    len = sizeof(kbuf);
  }

  if (copy_from_user(kbuf, buf, len)) {
    return -EFAULT;
  }

  mutex_lock(&iot_mutex);

  if (kfifo_avail(&iot_fifo) < len) {
    mutex_unlock(&iot_mutex);

    pr_err("iot_uart: FIFO full\n");

    return -ENOSPC;
  }

  // For demonstration, we simply write the data back into the FIFO buffer.
  kfifo_in(&iot_fifo, kbuf, len);

  // Wake up any waiting readers
  wake_up_interruptible(&iot_wait_queue);

  mutex_unlock(&iot_mutex);

  pr_info("iot_uart: wrote %zu bytes into FIFO\n", len);

  return len;
}

// poll function - called to check the status of the device for asynchronous I/O
static __poll_t iot_poll(struct file *file, poll_table *wait) {
  __poll_t mask = 0;

  poll_wait(file, &iot_wait_queue, wait);

  if (!kfifo_is_empty(&iot_fifo)) {
    mask |= POLLIN | POLLRDNORM;
  }

  return mask;
}

// UART receive thread function - continuously reads data from the UART device
// and stores it in the FIFO buffer
static int uart_rx_thread(void *data) {
  
  char rx_buf[UART_BUFFER_SIZE];
  char parsed_frame[FRAME_MAX_SIZE];
  int bytes_read;

  static char stream_buffer[STREAM_BUFFER_SIZE];
  static int stream_len = 0;

  pr_info("iot_uart: UART RX thread started\n");

  // Open ttyAMA0 for reading
  uart_filp = filp_open(UART_DEVICE, O_RDONLY, 0);

  if (IS_ERR(uart_filp)) 
  {
    pr_err("iot_uart: failed to open %s\n", UART_DEVICE);

    return PTR_ERR(uart_filp);
  }

  pr_info("iot_uart: opened %s\n", UART_DEVICE);

  while (!kthread_should_stop()) 
  {
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(parsed_frame, 0, sizeof(parsed_frame));

    bytes_read = kernel_read(uart_filp, rx_buf, sizeof(rx_buf) - 1, NULL);
    pr_info("iot_uart: kernel_read returned %d bytes\n", bytes_read);

    if (bytes_read > 0) 
    {
      memcpy(stream_buffer + stream_len, rx_buf, bytes_read);
      stream_len += bytes_read;
      stream_buffer[stream_len] = '\0';

      pr_info("iot_uart: RX raw stream: %s\n", stream_buffer);

      if (parse_uart_stream(stream_buffer, stream_len, parsed_frame)) 
      {
        if (validate_uart_frame(parsed_frame)) 
        {
          memset(stream_buffer, 0, sizeof(stream_buffer));
          stream_len = 0;

          mutex_lock(&iot_mutex);

          size_t frame_len = strnlen(parsed_frame, FRAME_MAX_SIZE);

          if (kfifo_avail(&iot_fifo) >= frame_len) 
          {
            kfifo_in(&iot_fifo, parsed_frame, frame_len);

            wake_up_interruptible(&iot_wait_queue);

            pr_info("iot_uart: validated frame buffered: %s\n", parsed_frame);
          } 
          else 
          {
            pr_err("iot_uart: FIFO full dropping frame\n");
          }

          mutex_unlock(&iot_mutex);
        } 
        else 
        {
          pr_err("iot_uart: CRC validation failed dropping frame\n");
        }
      } 
      else 
      {
        pr_info("iot_uart: incomplete frame received\n");
      }
    }
    else if (bytes_read == -EAGAIN)
    {
       msleep(20);
       continue;
    }

    msleep(100);
  }

  filp_close(uart_filp, NULL);

  pr_info("iot_uart: UART RX thread stopped\n");

  return 0;
}

// Module initialization function - called when the module is loaded
static int __init iot_driver_init(void) {
  int ret;

  pr_info("iot_uart: initializing driver\n");

  // Allocate device number - dynamically allocate a major number
  ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
  if (ret < 0) {
    pr_err("iot_uart: alloc_chrdev_region failed\n");
    return ret;
  }

  // Log the allocated major and minor numbers
  pr_info("iot_uart: major=%d minor=%d\n", MAJOR(dev_num), MINOR(dev_num));

  // Initialize cdev - set up the character device structure
  cdev_init(&iot_cdev, &fops);

  // Add cdev to kernel - register the character device with the kernel
  ret = cdev_add(&iot_cdev, dev_num, 1);
  if (ret < 0) {
    pr_err("iot_uart: cdev_add failed\n");
    unregister_chrdev_region(dev_num, 1);
    return ret;
  }

  // Create device class - create a device class for the driver
  iot_class = class_create(CLASS_NAME);
  if (IS_ERR(iot_class)) {
    pr_err("iot_uart: class_create failed\n");

    cdev_del(&iot_cdev); // Clean up cdev before returning
    unregister_chrdev_region(dev_num,
                             1); // Clean up device number before returning

    return PTR_ERR(iot_class); // Return the error code from class_create
  }

  // Create device node - create a device node in /dev
  if (IS_ERR(device_create(iot_class, NULL, dev_num, NULL, DEVICE_NAME))) {
    pr_err("iot_uart: device_create failed\n");

    class_destroy(iot_class); // Clean up class before returning
    cdev_del(&iot_cdev);      // Clean up cdev before returning
    unregister_chrdev_region(dev_num,
                             1); // Clean up device number before returning

    return -1;
  }

  mutex_init(&iot_mutex); // Initialize the mutex for synchronizing access to
                          // the FIFO buffer

  INIT_KFIFO(iot_fifo); // Initialize the FIFO buffer

  pr_info("iot_uart: buffered driver loaded\n");

  // Start the UART receive thread
  uart_thread = kthread_run(uart_rx_thread, NULL, "iot_uart_rx");

  if (IS_ERR(uart_thread)) {
    pr_err("iot_uart: failed to create UART thread\n");

    device_destroy(iot_class, dev_num);
    class_destroy(iot_class);
    cdev_del(&iot_cdev);
    unregister_chrdev_region(dev_num, 1);

    return PTR_ERR(uart_thread);
  }

  return 0;
}

// Module exit function - called when the module is unloaded
static void __exit iot_driver_exit(void) {
  // Stop the UART receive thread
  if (uart_thread) {
    kthread_stop(uart_thread);
  }

  device_destroy(iot_class, dev_num); // Remove the device node
  class_destroy(iot_class);           // Destroy the device class
  cdev_del(&iot_cdev); // Remove the character device from the kernel
  unregister_chrdev_region(dev_num, 1); // Unregister the device number

  mutex_destroy(&iot_mutex); // Destroy the mutex

  pr_info("iot_uart: driver unloaded\n");
}

module_init(iot_driver_init);
module_exit(iot_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishwajit Kumar Tiwari");
MODULE_DESCRIPTION("Buffered Hybrid Linux-RTOS IoT UART Driver");