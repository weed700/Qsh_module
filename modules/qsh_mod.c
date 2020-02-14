#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/quota.h>
#include <linux/dqblk_xfs.h>

//system call
#include <linux/syscalls.h>
#include <asm/unistd.h>

struct task_struct* g_th_id=NULL;


static int kthread_ex(void* arg)
{
    struct fs_disk_quota test_disk;

    printk(KERN_ALERT "Q_sh : %s() : called \n", __FUNCTION__);
    
    while(!kthread_should_stop())
    {
        printk(KERN_ALERT "Q_sh : %s() : loop => d_rt_spc_hardlimit : %u \n", __FUNCTION__,test_disk.d_id);
        ssleep(1);
    }

    printk(KERN_ALERT "Q_sh : %s() : kthread_should_stop() called. Bye.\n", __FUNCTION__);
    return 0;
}


static int __init hello_world_init(void)
{
    printk(KERN_ALERT "Q_sh : %s() : called\n", __FUNCTION__);

    /*
    printk(KERN_INFO "system call table : %p \n",sys_call_table);
    ref_sys_quotactl = (void *)sys_call_table[__NR_quotactl];
    sys_call_table[__NR_quotactl] = (unsigned long *)new_syscall;
    */


    if(g_th_id == NULL)
    {
        g_th_id = (struct task_struct *)kthread_run(kthread_ex, NULL, "kthread_example");
    }

    return 0;
}

static void __exit hello_world_finish(void)
{
    //sys_call_table[__NR_quotactl] = (unsigned long *)ref_sys_quotactl;

    if(g_th_id)
    {
        kthread_stop(g_th_id);
        g_th_id = NULL;
    }
    printk(KERN_ALERT "Q_sh : %s() : Bye.\n", __FUNCTION__);
}

module_init(hello_world_init);
module_exit(hello_world_finish);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sehoon<weed700@gmail.com>");
MODULE_DESCRIPTION("Hello world for Linux Kernel module");
