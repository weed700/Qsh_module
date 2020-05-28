#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/fs.h>


static char read_file (char *filename)
{

    struct file* fd = NULL;
    char buf;
    loff_t pos = 0;
    //struct file *file;
    mm_segment_t old_fs = get_fs();
    int err = 0;

    set_fs(KERNEL_DS);
    fd = filp_open(filename, O_RDONLY, 0);

    if(IS_ERR(fd)){
        err = PTR_ERR(fd);
        return 0;
    }
    if(fd)
    {
        //file = fget(fd);
        kernel_read(fd, &buf, sizeof(buf), &pos);
        printk("buf is %c\n", buf);

        //sys_close(fd);
        filp_close(fd,NULL);

    }

    
    if('0' == buf)
        printk("0 == buf\n");
    else
        printk("1 == buf\n");
    
    set_fs(old_fs);

    return buf;
}




static int __init init(void)
{
    char qsh_flag;

    printk("<0> file handling start\n");
    qsh_flag = read_file("/root/.meta");
    //write_file (“/etc/temp”, “foo”);
    printk("<0> file handing end : %c\n",qsh_flag);



    return 0;

}



static void __exit t_exit(void)
{

    printk("<0> bye\n");

}



MODULE_LICENSE("GPL");

module_init(init);

module_exit(t_exit);

