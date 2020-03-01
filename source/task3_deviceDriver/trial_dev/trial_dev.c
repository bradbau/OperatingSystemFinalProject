
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define trialMEM_SIZE 0x4000  /* 8KB for trial driver */
#define GLOBAL_MAJOR 500  /* Set device major */
#define MEM_CLEAR 1  /* Use for ioctl to clear memory */

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("A simple character driver module");

struct trial_dev {
  struct cdev cdev;
  unsigned char mem[trialMEM_SIZE];
};

static struct trial_dev *trial_devp;  /* Global dev pointer */
struct device *device;
struct class *class;

/*
 * trialdriver_open - Open the driver
 */
static int trialdriver_open(struct inode *inodep, struct file *filep) {
  printk(KERN_INFO "trial Driver: open\n");
  filep->private_data = trial_devp;
  return 0;
}

/*
 * trialdriver_release - Release the driver
 */
static int trialdriver_release(struct inode *inodep, struct file *filep) {
  printk(KERN_INFO "trial Driver: release\n");
  return 0;
}

/*
 * trialdriver_read - Read from the driver
 */
static ssize_t trialdriver_read(struct file *filep, char __user *buf, size_t count, loff_t *offset) {
  printk(KERN_INFO "trial Driver: start read\n");

  int ret = 0;
  size_t avail = trialMEM_SIZE - *offset;
  struct trial_dev *dev = filep->private_data;

  /* Available memory exists */
  if (count <= avail) {
    if (copy_to_user(buf, dev->mem + *offset, count) != 0)
      return -EFAULT;
    *offset += count;
    ret = count;
  }
  /* Available memory not enough */
  else {
    if (copy_to_user(buf, dev->mem + *offset, avail) != 0)
      return -EFAULT;
    *offset += avail;
    ret = avail;
  }

  printk(KERN_INFO "trial Driver: read %u bytes\n", ret);
  return ret;
}

/*
 * trialdriver_write - Write from the driver
 */
static ssize_t trialdriver_write(struct file *filep, const char __user *buf, size_t count, loff_t *offset) {
  printk(KERN_INFO "trial Driver: start write\n");

  int ret = 0;
  size_t avail = trialMEM_SIZE - *offset;
  struct trial_dev *dev = filep->private_data;
  memset(dev->mem + *offset, 0, avail);
  printk(KERN_INFO "trial Driver: After write\n");

  /* Available memory exists */
  if (count > avail) {
    if (copy_from_user(dev->mem + *offset, buf, avail) != 0)
      return -EFAULT;
    *offset += avail;
    ret = avail;
  }
  /* Available memory not enough */
  else {
    if (copy_from_user(dev->mem + *offset, buf, count) != 0)
      return -EFAULT;
    *offset += count;
    ret = count;
  }

  printk(KERN_INFO "trial Driver: written %u bytes\n", ret);
  return ret;
}

/*
 * trialdriver_llseek - Set the current position of the file for reading and writing
 */
static loff_t trialdriver_llseek(struct file *filep, loff_t offset, int whence) {
  printk(KERN_INFO "trial Driver: start llseek\n");

  loff_t ret = 0;
  switch (whence) {
  /* SEEK_SET */
  case 0:
    if (offset < 0) {
      ret = -EINVAL;
      break;
    }
    if (offset > trialMEM_SIZE) {
      ret = -EINVAL;
      break;
    }
    ret = offset;
    break;
  /* SEEK_CUR*/
  case 1:
    if ((filep->f_pos + offset) > trialMEM_SIZE) {
      ret = -EINVAL;
      break;
    }
    if ((filep->f_pos + offset) < 0) {
      ret = -EINVAL;
      break;
    }
    ret = filep->f_pos + offset;
    break;
  /* 
   * SEEK_END: Here we can't use SEEK_END,
   *           beacuse the memory is solid.
   *
  case 2:
    if (offset < 0) {
      ret = -EINVAL;
      break;
    }
    ret = trialMEM_SIZE + offset;
    break;*/
  /* Else: return error */
  default:
    ret = -EINVAL;
  }

  if (ret < 0)
    return ret;

  printk(KERN_INFO "trial Driver: set offset to %u\n", ret);
  filep->f_pos = ret;
  return ret;
}

/*
 * trialdriver_ioctl - Control the trial driver(memory clear)
 */
static long trialdriver_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
  printk(KERN_INFO "trial Driver: start memory clear\n");

  struct trial_dev *dev = filep->private_data;
  switch (cmd) {
  case MEM_CLEAR:
    memset(dev->mem, 0, trialMEM_SIZE);
    printk("trial Driver: memory is set to zero\n");
    break;
  default:
    return -EINVAL;
  }
  return 0;
}

/*
 * Set operation pointers
 */
static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = trialdriver_open,
  .release = trialdriver_release,
  .read = trialdriver_read,
  .write = trialdriver_write,
  .llseek = trialdriver_llseek,
  .unlocked_ioctl = trialdriver_ioctl,
};

/*
 * trialdriver_init - Initial function for trialdriver
 */
static int __init trialdriver_init(void) {
  printk(KERN_INFO "Load module: trialdriver\n");

  int ret;
  dev_t devno = MKDEV(GLOBAL_MAJOR, 0);
  ret = register_chrdev_region(devno, 1, "trialdriver");
  if (ret < 0) {
    printk(KERN_ALERT "Registering the character device failed with %d\n", ret);
    return ret;
  }

  /* Alloc memory for device */
  trial_devp = kzalloc(sizeof(struct trial_dev), GFP_KERNEL);
  if (trial_devp == NULL) {
    printk(KERN_ALERT "Alloc memory for device failed\n");
    ret = -ENOMEM;
    goto failed;
  }
  memset(trial_devp->mem, 0, trialMEM_SIZE);

  /* Setup device */
  cdev_init(&trial_devp->cdev, &fops);
  trial_devp->cdev.owner = THIS_MODULE;
  cdev_add(&trial_devp->cdev, devno, 1);

  /* Creat device file */
  class = class_create(THIS_MODULE, "trialdriver");
  if (IS_ERR(class)) {
    ret = PTR_ERR(class);
    printk(KERN_ALERT "Creat class for device file failed with %d\n", ret);
    goto failed;
  }
  device = device_create(class, NULL, devno, NULL, "trialdriver");
  if (IS_ERR(device)) {
    class_destroy(class);
    ret = PTR_ERR(device);
    printk(KERN_ALERT "Creat device file failed with %d\n", ret);
    goto failed;
  }

  return 0;

 failed:
  unregister_chrdev_region(devno, 1);
  return ret;
}

/*
 * trialdriver_exit - Exit function for trialdriver
 */
static void __exit trialdriver_exit(void) {
  printk(KERN_INFO "Unload module: trialdriver\n");

  device_destroy(class, MKDEV(GLOBAL_MAJOR, 0));
  class_unregister(class);
  class_destroy(class);

  cdev_del(&trial_devp->cdev);
  kfree(trial_devp);
  unregister_chrdev_region(MKDEV(GLOBAL_MAJOR, 0), 1);
}

module_init(trialdriver_init);
module_exit(trialdriver_exit);
