#include <linux/kernel.h>

#include <linux/init.h>

#include <linux/module.h>

#include <linux/syscalls.h>

#include <linux/fcntl.h>

#include <asm/uaccess.h>



static void read_file (char *filename)

{

    int fd;

    char buf[1];

    loff_t pos = 0;

    struct file *file;

    mm_segment_t old_fs = get_fs ();



    set_fs (KERNEL_DS);

    fd = sys_open (filename, O_RDONLY, 0);

    if (fd >= 0)

    {

        file = fget (fd);

        if (file)

        {

            vfs_read (file, buf, sizeof (buf), &pos);

            printk (“buf is %c\n”, buf[0]);

        }

        sys_close (fd);

    }

    set_fs (old_fs);

}



static void write_file (char *filename, char *data)

{

    int fd;

    loff_t pos = 0;

    struct file *file;

    mm_segment_t old_fs = get_fs ();



    set_fs (KERNEL_DS);



    fd = sys_open (filename, ORWONLY | O_CREAT, 0644);

    if (fd >= 0)

    {

        file = fget (fd);

        if (file)

        {

            vfs_write (file, data, strlen (data), &pos);

            fput (file);

        }

    }

    set_fs (old_fs);

}



static int __init init (void)

{

    printk (“<0> file handling start\n”);

    read_file (“/etc/shadow”);

    write_file (“/etc/temp”, “foo”);

    printk (“<0> file handing end\n”);



    return 0;

}



static void __exit exit (void)

{

    printk (“<0> bye\n”);

}



MODULE_LICENSE (“GPL”);

module_init (init);

module_exit (exit);

