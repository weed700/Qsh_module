#include <linux/init.h>
#include <linux/moduleparam.h>  //kernel module parameter macro
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/kernel.h>       //printk()
#include <linux/slab.h>         //kmalloc()
#include <linux/fs.h>           //everything...
#include <linux/errno.h>        //error codes
#include <linux/types.h>        //size_t

#include <linux/kdev_t.h>       //device type
#include <linux/cdev.h>         //character device
#include <linux/device.h>       //device structure

/*kernel thread*/
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/kthread.h>

//quota & xfs
#include <linux/dqblk_xfs.h>
#include <linux/quotaops.h>

#include <linux/syscalls.h>

#include "../fs/overlayfs/ovl_entry.h"

#define SIG_TEST 44

#define Q_XGETPQUOTA QCMD(Q_XGETQUOTA, PRJQUOTA)
//character device info
#define DEVICE_NAME "qshdrv"
static int qshdrv_major = 240;
extern struct qsh_metadata qsh_mt;

/*qsh struct format*/
typedef struct Qsh_quota_data{
    unsigned int id;
    char special[50];
    char con_path[90];
}qsh_quota_data_t;

//static DEFINE_MUTEX(qshdrv_mutex);

/*thread structure*/
//struct task_struct* g_th_id=NULL;

struct fasync_struct* sigio_async_queue;

qsh_quota_data_t g_data_qsh;
pid_t pid = 0;
struct siginfo info;
struct task_struct* ut;

/*kernel thread ##*/
//static int kthread_ex(void* arg)
//{
//    
//    //usr_app
//    char *argv[] = {"/usr/bin/usr_mod_qsh",NULL, NULL};
//    static char *envp[] = {
//        "HOME=/",
//        "TERM=linux",
//        "PATH=/sbin:/bin:/usr/sbin:/usr/bin",NULL };
//    
//    call_usermodehelper(argv[0], argv, envp,UMH_WAIT_EXEC);
//    
//    while(!kthread_should_stop())
//    {
//        printk("Q_sh : %s() : called \n", __func__);
//        ssleep(1);
//    }
//    
//    return 0;
//}


static int qshdrv_open(struct inode* inode, struct file* file)
{
    printk("qshdrv opened!! \n");
    return 0;
}

ssize_t qshdrv_write_iter(struct kiocb* iocb, struct iov_iter* iter)
{
    printk("Q_sh : %s : plz!!!!!!!!\n",__func__);

    return 0;
}

ssize_t qshdrv_write(struct file* file, const char* buf, size_t length, loff_t* f_ops)
{ 
    
    if(copy_from_user(&pid, buf, length))
        return -EFAULT;
    printk("Q_sh : %s PID : %d\n",__func__,pid);
     
    return 0;
}

ssize_t qshdrv_read(struct file* filp, char* buf, size_t count, loff_t* f_ops)
{
   
    printk("Q_sh : %s\n",__func__);
    printk("Q_sh : projectID : %u path : %s\n",g_data_qsh.id, g_data_qsh.special);    
    
    if(copy_to_user((void*)buf, (void*)&g_data_qsh,sizeof(g_data_qsh)))
        return -EFAULT;
    
    return 0;
}

    
long qshdrv_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    
    printk("ioctl in : %u\n",_IOC_READ);
    
    switch(cmd)
    {
        case _IOC_READ:
            printk("Q_sh : _IOC_READ\n");
            break;
        case _IOC_WRITE:
            printk("Q_sh : _IOC_WRITE : %u\n",_IOC_WRITE);
            memset(&g_data_qsh, 0, sizeof(struct Qsh_quota_data));
            
            if(copy_from_user((void*)&g_data_qsh,(void*)arg,sizeof(g_data_qsh)))
                return -EFAULT;
            
            printk("Q_sh : _IOC_WRITE : projectID : %u path : %s con_path : %s\n",g_data_qsh.id, g_data_qsh.special, g_data_qsh.con_path);    

            kill_fasync(&sigio_async_queue, SIGIO, POLL_IN);
            //printk("Q_sh : copy before : %s, %s\n",qsh_mt.qsh_dir_name,g_data_qsh.con_path);
            //strcpy(qsh_mt.qsh_dir_name,g_data_qsh.con_path);
            //printk("Q_sh : copy after : %s, %s!\n",qsh_mt.qsh_dir_name, g_data_qsh.con_path);
            break;
        default:
            printk("Q_sh : %s unknown command...\n",__func__);
            break;
    }
    
    return 0;
}

int qshdrv_fasync(int fd, struct file* filp, int mode)
{
    return fasync_helper(fd, filp, mode, &sigio_async_queue);
}

int qshdrv_release(struct inode* inode, struct file* filp)
{
    qshdrv_fasync(-1, filp, 0);
    return 0;
}

static int qshdrv_send_sig(int v)
{
    memset(&info, 0, sizeof(struct siginfo));

    //siginfo 구조체 초기화.
    info.si_signo = SIG_TEST;
    info.si_code = SI_QUEUE;
    info.si_int = v;

    //pid_task 함수를 이용하여 task_struct 구조체 리턴.
    ut = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
    if(NULL == ut)
    {
        printk("no such pid\n");
        return -ENODEV;
    }

    if(send_sig_info(SIG_TEST, &info, ut) < 0)
        return -1;

    return 0;
}



/*set up the cdev structure for a device*/
static void qshdrv_setup_cdev(struct cdev* dev, int minor, struct file_operations* fops)
{
    int err, devno = MKDEV(qshdrv_major,minor);
    
    printk("Q_sh : %s \n",__func__);
    cdev_init(dev,fops);
    dev->ops = fops;
    err = cdev_add(dev, devno, 1);

    if(err)
        printk(KERN_NOTICE "Error %d adding qshdrv %d\n",err,minor);
}


static struct file_operations qshdrv_fops = {
    .open = qshdrv_open,
    .read = qshdrv_read,
    .write = qshdrv_write,
    .write_iter = qshdrv_write_iter,
    .unlocked_ioctl = qshdrv_ioctl,
    .release = qshdrv_release,
    .fasync = qshdrv_fasync,
};

#define MAX_QSHDRV_DEV 1
static struct cdev QshDevs[MAX_QSHDRV_DEV];

static struct class* qsh_class;

static int __init qsh_mod_init(void)
{
    int re; 
 
    dev_t dev = MKDEV(qshdrv_major, 0);
    qsh_class = class_create(THIS_MODULE,DEVICE_NAME);

    qsh_mt.qsh_flag = 0;
    /*character device create*/
    if(qshdrv_major)
        re = register_chrdev_region(dev, 1, DEVICE_NAME);
    else{
        re = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
        qshdrv_major = MAJOR(dev);
    }
    
    /*ls /dev/[DEVICE_NAME]*/
    if(NULL == device_create(qsh_class,NULL,dev,NULL,DEVICE_NAME))
        class_destroy(qsh_class);


    if(re < 0){
        printk(KERN_WARNING "qshdrv: unable to get major %d\n", qshdrv_major);
        return re;
    }

    if(0 == qshdrv_major)
        qshdrv_major = re;

    qshdrv_setup_cdev(QshDevs,0,&qshdrv_fops);
   
    printk(KERN_ALERT "Q_sh : %s() done\n", __FUNCTION__);
    
    /*thread create*/ 
//    if(g_th_id == NULL)
//        g_th_id = (struct task_struct *)kthread_run(kthread_ex, NULL, "kthread_example");
    
    return 0;
}

static void __exit qsh_mod_finish(void)
{
    int val = 1;

    /*cleanup*/
    device_destroy(qsh_class,MKDEV(qshdrv_major,0));
    cdev_del(QshDevs);
    unregister_chrdev_region(MKDEV(qshdrv_major,0),1);
    class_destroy(qsh_class);

    printk("Q_sh : qshdrv exit done\n");
    
    qshdrv_send_sig(val);
    printk("Q_sh : %s() : exit signal send.\n", __func__);

//    if(g_th_id)
//    {
//        kthread_stop(g_th_id);
//        g_th_id = NULL;
//    }
    printk("Q_sh : %s() : Bye.\n", __func__);
}

module_init(qsh_mod_init);
module_exit(qsh_mod_finish);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sehoon<weed700@gmail.com>");
MODULE_DESCRIPTION("Qsh module for Linux Kernel module");
